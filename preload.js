const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electronAPI', {
  // VS Code operations
  checkVSCode: () => ipcRenderer.invoke('check-vscode'),
  getVSCodePath: () => ipcRenderer.invoke('get-vscode-path'),
  setVSCodePath: (path) => ipcRenderer.invoke('set-vscode-path', path),
  browseVSCode: () => ipcRenderer.invoke('browse-vscode'),
  refreshVSCode: () => ipcRenderer.invoke('refresh-vscode'),
  
  // API Key operations
  saveApiKey: (key) => ipcRenderer.invoke('save-api-key', key),
  
  // Server operations
  startServer: () => ipcRenderer.invoke('start-server'),
  stopServer: () => ipcRenderer.invoke('stop-server'),
  getServerStatus: () => ipcRenderer.invoke('get-server-status'),
  
  // UI operations
  openExternalLink: (url) => ipcRenderer.invoke('open-external-link', url),
  getEncryptionStatus: () => ipcRenderer.invoke('get-encryption-status'),
  
  // Listeners
  onVSCodeStatus: (callback) => {
    ipcRenderer.on('vscode-status', (event, data) => callback(data));
  },
  onShowVSCodePrompt: (callback) => {
    ipcRenderer.on('show-vscode-prompt', (event, data) => callback(data));
  },
  onShowApiKeyDialog: (callback) => {
    ipcRenderer.on('show-api-key-dialog', (event, data) => callback(data));
  },
  onApiKeyValid: (callback) => {
    ipcRenderer.on('api-key-valid', (event, data) => callback(data));
  },
  onServerStarted: (callback) => {
    ipcRenderer.on('server-started', (event, data) => callback(data));
  },
  onServerStopped: (callback) => {
    ipcRenderer.on('server-stopped', (event, data) => callback(data));
  },
  onServerError: (callback) => {
    ipcRenderer.on('server-error', (event, data) => callback(data));
  }
});