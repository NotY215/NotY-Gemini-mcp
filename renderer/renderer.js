// Application state
const state = {
  vscodeInstalled: false,
  vscodePath: null,
  apiKeyValid: false,
  serverRunning: false,
  isVerifying: false,
  isStartingServer: false
};

// DOM Elements
const elements = {
  vscodeSection: document.getElementById('vscodeSection'),
  apiKeySection: document.getElementById('apiKeySection'),
  serverSection: document.getElementById('serverSection'),
  
  vscodeStatus: document.getElementById('vscodeStatus'),
  vscodeIndicator: document.getElementById('vscodeIndicator'),
  vscodeStatusText: document.getElementById('vscodeStatusText'),
  vscodePathDisplay: document.getElementById('vscodePathDisplay'),
  
  browseVSCodeBtn: document.getElementById('browseVSCodeBtn'),
  refreshVSCodeBtn: document.getElementById('refreshVSCodeBtn'),
  downloadVSCodeBtn: document.getElementById('downloadVSCodeBtn'),
  
  apiKeyInput: document.getElementById('apiKeyInput'),
  toggleKeyVisibility: document.getElementById('toggleKeyVisibility'),
  saveApiKeyBtn: document.getElementById('saveApiKeyBtn'),
  getApiKeyBtn: document.getElementById('getApiKeyBtn'),
  apiKeyStatus: document.getElementById('apiKeyStatus'),
  
  startServerBtn: document.getElementById('startServerBtn'),
  stopServerBtn: document.getElementById('stopServerBtn'),
  serverStatusText: document.getElementById('serverStatusText'),
  serverStatus: document.getElementById('serverStatus'),
  
  logArea: document.getElementById('logArea'),
  toast: document.getElementById('toast'),
  
  viewTermsLink: document.getElementById('viewTermsLink')
};

// Initialize application
document.addEventListener('DOMContentLoaded', () => {
  setupEventListeners();
  checkInitialStatus();
});

// Setup event listeners
function setupEventListeners() {
  // VS Code operations
  elements.browseVSCodeBtn.addEventListener('click', browseVSCode);
  elements.refreshVSCodeBtn.addEventListener('click', refreshVSCode);
  elements.downloadVSCodeBtn.addEventListener('click', downloadVSCode);
  
  // API Key operations
  elements.saveApiKeyBtn.addEventListener('click', saveApiKey);
  elements.getApiKeyBtn.addEventListener('click', getApiKey);
  elements.toggleKeyVisibility.addEventListener('click', toggleKeyVisibility);
  
  // Server operations
  elements.startServerBtn.addEventListener('click', startServer);
  elements.stopServerBtn.addEventListener('click', stopServer);
  
  // Terms and conditions
  elements.viewTermsLink.addEventListener('click', viewTerms);
  
  // Enter key for API key input
  elements.apiKeyInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') {
      saveApiKey();
    }
  });
}

// Check initial status
async function checkInitialStatus() {
  try {
    // Check encryption and stored data
    const status = await window.electronAPI.getEncryptionStatus();
    
    if (status.termsAccepted) {
      // Check VS Code
      await refreshVSCode();
    }
    
    if (status.vscodePath) {
      elements.vscodePathDisplay.textContent = `VS Code Path: ${status.vscodePath}`;
    }
    
    if (status.hasKey) {
      elements.apiKeySection.style.display = 'block';
      elements.apiKeyInput.value = '••••••••••••••••';
      elements.saveApiKeyBtn.textContent = '✓ Verify Key';
    }
    
    if (status.serverRunning) {
      updateServerStatus(true);
    }
    
    // Listen for events from main process
    setupEventListenersFromMain();
  } catch (error) {
    console.error('Error checking initial status:', error);
    addLog('Error initializing application: ' + error.message, 'error');
  }
}

// Setup event listeners from main process
function setupEventListenersFromMain() {
  window.electronAPI.onVSCodeStatus((data) => {
    updateVSCodeStatus(data.installed, data.path);
  });
  
  window.electronAPI.onShowVSCodePrompt((show) => {
    if (show) {
      elements.vscodeSection.style.display = 'block';
      elements.apiKeySection.style.display = 'none';
      elements.serverSection.style.display = 'none';
    }
  });
  
  window.electronAPI.onShowApiKeyDialog((show) => {
    if (show && state.vscodeInstalled) {
      elements.apiKeySection.style.display = 'block';
      elements.serverSection.style.display = 'none';
    }
  });
  
  window.electronAPI.onApiKeyValid((valid) => {
    state.apiKeyValid = valid;
    if (valid) {
      elements.serverSection.style.display = 'block';
      elements.saveApiKeyBtn.textContent = '✓ Verified';
      elements.saveApiKeyBtn.disabled = true;
      elements.apiKeyInput.disabled = true;
      showToast('API Key verified successfully!', 'success');
      addLog('✅ API Key verified successfully');
    } else {
      elements.apiKeyInput.value = '';
      elements.apiKeyInput.disabled = false;
      elements.saveApiKeyBtn.disabled = false;
      elements.apiKeyStatus.innerHTML = '<span style="color: #f56565;">❌ Invalid API key. Please try again.</span>';
      showToast('Invalid API key. Please check and try again.', 'error');
    }
  });
  
  window.electronAPI.onServerStarted(() => {
    updateServerStatus(true);
    showToast('Server started successfully!', 'success');
    addLog('🚀 Server started successfully');
  });
  
  window.electronAPI.onServerStopped(() => {
    updateServerStatus(false);
    showToast('Server stopped.', 'info');
    addLog('⏹️ Server stopped');
  });
  
  window.electronAPI.onServerError((error) => {
    showToast('Server error: ' + error, 'error');
    addLog('❌ Server error: ' + error, 'error');
    updateServerStatus(false);
  });
}

// Update VS Code status
function updateVSCodeStatus(installed, path) {
  state.vscodeInstalled = installed;
  state.vscodePath = path;
  
  if (installed) {
    elements.vscodeIndicator.className = 'status-indicator installed';
    elements.vscodeStatusText.textContent = `✅ VS Code is installed${path ? ` at: ${path}` : ''}`;
    if (path) {
      elements.vscodePathDisplay.textContent = `VS Code Path: ${path}`;
    }
    elements.apiKeySection.style.display = 'block';
  } else {
    elements.vscodeIndicator.className = 'status-indicator not-installed';
    elements.vscodeStatusText.textContent = '❌ VS Code is not installed';
    elements.vscodePathDisplay.textContent = '';
    elements.apiKeySection.style.display = 'none';
    elements.serverSection.style.display = 'none';
  }
}

// Browse for VS Code
async function browseVSCode() {
  try {
    const result = await window.electronAPI.browseVSCode();
    if (result.success) {
      await refreshVSCode();
      showToast('VS Code path updated successfully!', 'success');
      addLog(`📂 VS Code path set to: ${result.path}`);
    } else if (!result.cancelled) {
      showToast('Invalid VS Code executable selected. Please try again.', 'error');
    }
  } catch (error) {
    console.error('Error browsing VS Code:', error);
    showToast('Error browsing for VS Code', 'error');
  }
}

// Refresh VS Code detection
async function refreshVSCode() {
  try {
    addLog('🔄 Refreshing VS Code detection...');
    const result = await window.electronAPI.refreshVSCode();
    if (result.success) {
      const status = await window.electronAPI.getEncryptionStatus();
      if (status.vscodePath) {
        addLog(`✅ VS Code detected at: ${status.vscodePath}`);
      } else {
        addLog('⚠️ VS Code not found');
      }
    }
  } catch (error) {
    console.error('Error refreshing VS Code:', error);
    showToast('Error refreshing VS Code detection', 'error');
  }
}

// Download VS Code
function downloadVSCode() {
  window.electronAPI.openExternalLink('https://code.visualstudio.com/download?_exp_download=fb315fc982');
  addLog('📥 Opening VS Code download page...');
}

// Save API Key
async function saveApiKey() {
  const apiKey = elements.apiKeyInput.value.trim();
  
  if (!apiKey || apiKey === '••••••••••••••••') {
    showToast('Please enter a valid API key', 'error');
    return;
  }
  
  if (state.isVerifying) return;
  
  state.isVerifying = true;
  elements.saveApiKeyBtn.disabled = true;
  elements.saveApiKeyBtn.innerHTML = '<span class="loading-spinner"></span> Verifying...';
  elements.apiKeyStatus.innerHTML = '⏳ Verifying API key...';
  
  try {
    const result = await window.electronAPI.saveApiKey(apiKey);
    
    if (result.success && result.valid) {
      elements.apiKeyStatus.innerHTML = '<span style="color: #48bb78;">✅ API key verified and saved successfully!</span>';
      elements.apiKeyInput.value = '••••••••••••••••';
      elements.apiKeyInput.disabled = true;
      elements.saveApiKeyBtn.textContent = '✓ Verified';
      elements.saveApiKeyBtn.disabled = true;
      state.apiKeyValid = true;
      elements.serverSection.style.display = 'block';
      showToast('API key saved and verified!', 'success');
      addLog('✅ API key saved and verified');
    } else {
      elements.apiKeyStatus.innerHTML = `<span style="color: #f56565;">❌ ${result.message || 'Invalid API key. Please try again.'}</span>`;
      showToast('Invalid API key. Please check and try again.', 'error');
      elements.apiKeyInput.value = '';
      elements.apiKeyInput.disabled = false;
      elements.saveApiKeyBtn.textContent = '💾 Save & Verify Key';
      elements.saveApiKeyBtn.disabled = false;
    }
  } catch (error) {
    console.error('Error saving API key:', error);
    elements.apiKeyStatus.innerHTML = `<span style="color: #f56565;">❌ Error: ${error.message}</span>`;
    showToast('Error saving API key: ' + error.message, 'error');
    elements.saveApiKeyBtn.textContent = '💾 Save & Verify Key';
    elements.saveApiKeyBtn.disabled = false;
  }
  
  state.isVerifying = false;
}

// Get API Key (redirect to Google AI Studio)
function getApiKey() {
  window.electronAPI.openExternalLink('https://aistudio.google.com/api-keys');
  addLog('🔑 Opening Google AI Studio for API key...');
}

// Toggle API key visibility
function toggleKeyVisibility() {
  const input = elements.apiKeyInput;
  const button = elements.toggleKeyVisibility;
  
  if (input.type === 'password') {
    input.type = 'text';
    button.textContent = '🙈';
  } else {
    input.type = 'password';
    button.textContent = '👁️';
  }
}

// Start server
async function startServer() {
  if (state.isStartingServer) return;
  if (!state.apiKeyValid) {
    showToast('Please verify your API key first', 'error');
    return;
  }
  
  state.isStartingServer = true;
  elements.startServerBtn.disabled = true;
  elements.startServerBtn.innerHTML = '<span class="loading-spinner"></span> Starting...';
  
  try {
    const result = await window.electronAPI.startServer();
    if (result.success) {
      // Server started - status will be updated by event
      showToast('Server starting...', 'info');
      addLog('⏳ Server is starting...');
    } else {
      showToast('Failed to start server', 'error');
      addLog('❌ Failed to start server', 'error');
      elements.startServerBtn.disabled = false;
      elements.startServerBtn.textContent = '▶️ Start Server';
    }
  } catch (error) {
    console.error('Error starting server:', error);
    showToast('Error starting server: ' + error.message, 'error');
    addLog('❌ Error starting server: ' + error.message, 'error');
    elements.startServerBtn.disabled = false;
    elements.startServerBtn.textContent = '▶️ Start Server';
  }
  
  state.isStartingServer = false;
}

// Stop server
async function stopServer() {
  try {
    await window.electronAPI.stopServer();
    // Server stopped - status will be updated by event
  } catch (error) {
    console.error('Error stopping server:', error);
    showToast('Error stopping server: ' + error.message, 'error');
    addLog('❌ Error stopping server: ' + error.message, 'error');
  }
}

// Update server status
function updateServerStatus(running) {
  state.serverRunning = running;
  
  if (running) {
    elements.serverStatus.className = 'status-badge running';
    elements.serverStatus.textContent = '🟢 Server Running';
    elements.serverStatusText.textContent = '🟢 Running';
    elements.startServerBtn.disabled = true;
    elements.startServerBtn.textContent = '▶️ Running';
    elements.stopServerBtn.disabled = false;
    elements.apiKeyInput.disabled = true;
    elements.saveApiKeyBtn.disabled = true;
    addLog('🟢 Server is running. You can now use Gemini in VS Code terminal.');
  } else {
    elements.serverStatus.className = 'status-badge stopped';
    elements.serverStatus.textContent = '🔴 Server Stopped';
    elements.serverStatusText.textContent = '🔴 Stopped';
    elements.startServerBtn.disabled = false;
    elements.startServerBtn.textContent = '▶️ Start Server';
    elements.stopServerBtn.disabled = true;
    if (state.apiKeyValid) {
      elements.apiKeyInput.disabled = false;
      elements.saveApiKeyBtn.disabled = false;
    }
    addLog('🔴 Server stopped');
  }
}

// View Terms and Conditions
function viewTerms() {
  window.electronAPI.openExternalLink('about:blank');
  // In a real app, this would open the terms page
  showToast('Opening Terms & Conditions...', 'info');
  addLog('📄 Viewing Terms & Conditions');
}

// Show toast notification
function showToast(message, type = 'info') {
  const toast = elements.toast;
  toast.textContent = message;
  toast.className = `toast ${type} show`;
  
  clearTimeout(toast._timeout);
  toast._timeout = setTimeout(() => {
    toast.classList.remove('show');
  }, 5000);
}

// Add log entry
function addLog(message, type = 'info') {
  const logArea = elements.logArea;
  const entry = document.createElement('div');
  entry.className = `log-entry ${type}`;
  
  const timestamp = new Date().toLocaleTimeString();
  entry.textContent = `[${timestamp}] ${message}`;
  
  logArea.appendChild(entry);
  logArea.scrollTop = logArea.scrollHeight;
  
  // Limit log entries
  while (logArea.children.length > 100) {
    logArea.removeChild(logArea.firstChild);
  }
}