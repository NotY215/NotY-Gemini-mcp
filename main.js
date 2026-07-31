const { app, BrowserWindow, ipcMain, Tray, Menu, dialog, shell, nativeImage } = require('electron');
const path = require('path');
const Store = require('electron-store');
const { spawn } = require('child_process');
const fs = require('fs');
const os = require('os');

// Import backend modules
const VSCodeManager = require('./backend/vscode-manager');
const GeminiService = require('./backend/gemini-service');
const MCPServer = require('./backend/mcp-server');
const Encryption = require('./backend/encryption');

// Initialize store
const store = new Store({
  name: 'gemini-mcp-config',
  defaults: {
    apiKey: '',
    vscodePath: '',
    serverRunning: false,
    termsAccepted: false,
    lastTermsAccept: null
  }
});

let mainWindow = null;
let tray = null;
let mcpServer = null;
let geminiService = null;

// Encryption instance
const encryption = new Encryption();

// Create the main window
function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1000,
    height: 700,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js')
    },
    icon: path.join(__dirname, 'assets', 'icons', 'icon.png'),
    resizable: true,
    frame: true,
    show: false
  });

  // Load the main UI
  mainWindow.loadFile(path.join(__dirname, 'renderer', 'index.html'));

  // Show window when ready
  mainWindow.once('ready-to-show', () => {
    mainWindow.show();
    
    // Check if terms need to be shown
    if (!store.get('termsAccepted')) {
      showTermsDialog();
    }
    
    // Initial VS Code detection
    checkVSCodeInstallation();
  });

  mainWindow.on('closed', () => {
    mainWindow = null;
  });
}

// Show terms and conditions
function showTermsDialog() {
  const termsWindow = new BrowserWindow({
    width: 800,
    height: 600,
    parent: mainWindow,
    modal: true,
    show: false,
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true,
      preload: path.join(__dirname, 'preload.js')
    }
  });

  termsWindow.loadFile(path.join(__dirname, 'terms.html'));
  
  termsWindow.once('ready-to-show', () => {
    termsWindow.show();
  });

  // Listen for terms acceptance
  ipcMain.once('terms-accepted', (event, accepted) => {
    if (accepted) {
      store.set('termsAccepted', true);
      store.set('lastTermsAccept', new Date().toISOString());
      termsWindow.close();
      checkVSCodeInstallation();
    } else {
      app.quit();
    }
  });

  termsWindow.on('closed', () => {
    if (!store.get('termsAccepted')) {
      app.quit();
    }
  });
}

// Check VS Code installation
async function checkVSCodeInstallation() {
  const vscodeManager = new VSCodeManager();
  const isInstalled = await vscodeManager.checkInstallation();
  
  if (mainWindow && !mainWindow.isDestroyed()) {
    mainWindow.webContents.send('vscode-status', {
      installed: isInstalled,
      path: isInstalled ? await vscodeManager.getVSCodePath() : null
    });
  }

  // If VS Code is not installed, show the installation prompt
  if (!isInstalled) {
    if (mainWindow && !mainWindow.isDestroyed()) {
      mainWindow.webContents.send('show-vscode-prompt', true);
    }
  } else {
    // Check if API key exists
    const encryptedKey = store.get('apiKey');
    if (encryptedKey) {
      try {
        const apiKey = encryption.decrypt(encryptedKey);
        if (apiKey) {
          // Verify API key
          geminiService = new GeminiService(apiKey);
          const isValid = await geminiService.verifyKey();
          if (isValid) {
            mainWindow.webContents.send('api-key-valid', true);
            // Auto-start server if it was running before
            if (store.get('serverRunning')) {
              startMCPServer();
            }
          } else {
            mainWindow.webContents.send('api-key-valid', false);
            showApiKeyDialog();
          }
        }
      } catch (error) {
        console.error('Error decrypting API key:', error);
        showApiKeyDialog();
      }
    } else {
      showApiKeyDialog();
    }
  }
}

// Show API key dialog
function showApiKeyDialog() {
  if (mainWindow && !mainWindow.isDestroyed()) {
    mainWindow.webContents.send('show-api-key-dialog', true);
  }
}

// Start MCP Server
async function startMCPServer() {
  try {
    const encryptedKey = store.get('apiKey');
    if (!encryptedKey) {
      throw new Error('API key not found');
    }
    
    const apiKey = encryption.decrypt(encryptedKey);
    if (!apiKey) {
      throw new Error('Invalid API key');
    }

    // Initialize Gemini service
    geminiService = new GeminiService(apiKey);
    const isValid = await geminiService.verifyKey();
    
    if (!isValid) {
      throw new Error('Invalid API key');
    }

    // Get VS Code path
    const vscodeManager = new VSCodeManager();
    const vscodePath = await vscodeManager.getVSCodePath();
    
    if (!vscodePath) {
      throw new Error('VS Code not found');
    }

    // Start MCP server
    mcpServer = new MCPServer(geminiService, vscodePath);
    await mcpServer.start();
    
    store.set('serverRunning', true);
    
    // Create tray icon
    createTray();
    
    if (mainWindow && !mainWindow.isDestroyed()) {
      mainWindow.webContents.send('server-started', true);
    }
    
    console.log('MCP Server started successfully');
  } catch (error) {
    console.error('Failed to start MCP server:', error);
    if (mainWindow && !mainWindow.isDestroyed()) {
      mainWindow.webContents.send('server-error', error.message);
    }
  }
}

// Stop MCP Server
async function stopMCPServer() {
  if (mcpServer) {
    await mcpServer.stop();
    mcpServer = null;
  }
  
  store.set('serverRunning', false);
  
  if (mainWindow && !mainWindow.isDestroyed()) {
    mainWindow.webContents.send('server-stopped', true);
  }
  
  // Remove tray icon
  if (tray) {
    tray.destroy();
    tray = null;
  }
}

// Create tray icon
function createTray() {
  const iconPath = path.join(__dirname, 'assets', 'icons', 'tray-icon.png');
  let icon = nativeImage.createFromPath(iconPath);
  
  // Resize icon if needed
  if (icon.isEmpty()) {
    // Fallback: create a simple icon
    icon = nativeImage.createEmpty();
  }
  
  tray = new Tray(icon);
  
  const contextMenu = Menu.buildFromTemplate([
    {
      label: 'NotY-Gemini-MCP',
      enabled: false
    },
    {
      type: 'separator'
    },
    {
      label: 'Show App',
      click: () => {
        if (mainWindow) {
          mainWindow.show();
        }
      }
    },
    {
      label: 'Server Status: Running',
      enabled: false
    },
    {
      type: 'separator'
    },
    {
      label: 'Exit',
      click: () => {
        stopMCPServer();
        app.quit();
      }
    }
  ]);
  
  tray.setToolTip('NotY-Gemini-MCP - Server Running');
  tray.setContextMenu(contextMenu);
  
  tray.on('click', () => {
    if (mainWindow) {
      if (mainWindow.isVisible()) {
        mainWindow.hide();
      } else {
        mainWindow.show();
      }
    }
  });
}

// IPC handlers
ipcMain.handle('check-vscode', async () => {
  const vscodeManager = new VSCodeManager();
  return await vscodeManager.checkInstallation();
});

ipcMain.handle('get-vscode-path', async () => {
  const vscodeManager = new VSCodeManager();
  return await vscodeManager.getVSCodePath();
});

ipcMain.handle('set-vscode-path', async (event, path) => {
  const vscodeManager = new VSCodeManager();
  const isValid = await vscodeManager.validateVSCodeExecutable(path);
  if (isValid) {
    store.set('vscodePath', path);
    return true;
  }
  return false;
});

ipcMain.handle('save-api-key', async (event, apiKey) => {
  try {
    const encryptedKey = encryption.encrypt(apiKey);
    store.set('apiKey', encryptedKey);
    
    // Initialize and verify
    geminiService = new GeminiService(apiKey);
    const isValid = await geminiService.verifyKey();
    
    if (isValid) {
      return { success: true, valid: true };
    } else {
      store.set('apiKey', '');
      return { success: false, valid: false, message: 'Invalid API key' };
    }
  } catch (error) {
    return { success: false, valid: false, message: error.message };
  }
});

ipcMain.handle('start-server', async () => {
  await startMCPServer();
  return { success: true };
});

ipcMain.handle('stop-server', async () => {
  await stopMCPServer();
  return { success: true };
});

ipcMain.handle('get-server-status', () => {
  return store.get('serverRunning') || false;
});

ipcMain.handle('open-external-link', (event, url) => {
  shell.openExternal(url);
});

ipcMain.handle('browse-vscode', async () => {
  const result = await dialog.showOpenDialog(mainWindow, {
    properties: ['openFile'],
    filters: [
      { name: 'Executable', extensions: ['exe'] }
    ],
    title: 'Select VS Code Executable'
  });
  
  if (!result.canceled && result.filePaths.length > 0) {
    const filePath = result.filePaths[0];
    const vscodeManager = new VSCodeManager();
    const isValid = await vscodeManager.validateVSCodeExecutable(filePath);
    
    if (isValid) {
      store.set('vscodePath', filePath);
      return { success: true, path: filePath };
    } else {
      return { success: false, message: 'Invalid VS Code executable selected' };
    }
  }
  return { success: false, cancelled: true };
});

ipcMain.handle('refresh-vscode', async () => {
  await checkVSCodeInstallation();
  return { success: true };
});

ipcMain.handle('get-encryption-status', () => {
  const hasKey = store.get('apiKey') ? true : false;
  return {
    hasKey,
    termsAccepted: store.get('termsAccepted') || false,
    serverRunning: store.get('serverRunning') || false,
    vscodePath: store.get('vscodePath') || null
  };
});

// App lifecycle
app.whenReady().then(() => {
  createWindow();
});

app.on('window-all-closed', () => {
  // Keep app running in background if server is running
  if (!store.get('serverRunning')) {
    app.quit();
  }
});

app.on('before-quit', async () => {
  await stopMCPServer();
});