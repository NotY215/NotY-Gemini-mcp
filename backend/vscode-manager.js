const { exec } = require('child_process');
const { promisify } = require('util');
const fs = require('fs-extra');
const path = require('path');
const os = require('os');

const execAsync = promisify(exec);

class VSCodeManager {
  constructor() {
    this.vscodePaths = [];
    this.detectedPath = null;
  }

  async checkInstallation() {
    try {
      const possiblePaths = this.getPossibleVSCodePaths();
      
      for (const vscodePath of possiblePaths) {
        if (await fs.pathExists(vscodePath)) {
          this.detectedPath = vscodePath;
          return true;
        }
      }
      
      // Try using 'where' or 'which' command as fallback
      try {
        const { stdout } = await execAsync('where code 2>nul || which code 2>/dev/null');
        if (stdout.trim()) {
          const paths = stdout.trim().split('\n');
          for (const p of paths) {
            if (await fs.pathExists(p.trim())) {
              this.detectedPath = p.trim();
              return true;
            }
          }
        }
      } catch (error) {
        // 'where' or 'which' command not found or no output
      }
      
      return false;
    } catch (error) {
      console.error('Error checking VS Code installation:', error);
      return false;
    }
  }

  getPossibleVSCodePaths() {
    const paths = [];
    const platform = os.platform();
    
    if (platform === 'win32') {
      // Windows paths
      const programFiles = process.env['ProgramFiles'] || 'C:\\Program Files';
      const programFilesX86 = process.env['ProgramFiles(x86)'] || 'C:\\Program Files (x86)';
      const localAppData = process.env['LOCALAPPDATA'] || 
        path.join(os.homedir(), 'AppData', 'Local');
      
      paths.push(
        path.join(programFiles, 'Microsoft VS Code', 'Code.exe'),
        path.join(programFilesX86, 'Microsoft VS Code', 'Code.exe'),
        path.join(localAppData, 'Programs', 'Microsoft VS Code', 'Code.exe'),
        path.join(localAppData, 'Programs', 'Microsoft VS Code Insiders', 'Code - Insiders.exe')
      );
    } else if (platform === 'darwin') {
      // macOS paths
      paths.push(
        '/Applications/Visual Studio Code.app/Contents/Resources/app/bin/code',
        '/Applications/Visual Studio Code - Insiders.app/Contents/Resources/app/bin/code',
        path.join(os.homedir(), 'Applications', 'Visual Studio Code.app', 'Contents', 'Resources', 'app', 'bin', 'code')
      );
    } else {
      // Linux paths
      paths.push(
        '/usr/bin/code',
        '/usr/local/bin/code',
        path.join(os.homedir(), '.local', 'bin', 'code')
      );
    }
    
    return paths;
  }

  async getVSCodePath() {
    if (this.detectedPath && await fs.pathExists(this.detectedPath)) {
      return this.detectedPath;
    }
    
    const installed = await this.checkInstallation();
    if (installed) {
      return this.detectedPath;
    }
    
    return null;
  }

  async validateVSCodeExecutable(filePath) {
    try {
      // Check if file exists
      if (!await fs.pathExists(filePath)) {
        return false;
      }
      
      // Check if it's a VS Code executable
      const fileName = path.basename(filePath).toLowerCase();
      const isVSCode = fileName.includes('code') || fileName.includes('visual studio code');
      
      if (!isVSCode) {
        return false;
      }
      
      // On Windows, try to execute with --version to verify
      if (os.platform() === 'win32') {
        try {
          const { stdout } = await execAsync(`"${filePath}" --version`);
          if (stdout && stdout.includes('.')) {
            return true;
          }
        } catch (error) {
          return false;
        }
      }
      
      return true;
    } catch (error) {
      console.error('Error validating VS Code executable:', error);
      return false;
    }
  }

  async installVSCode() {
    const platform = os.platform();
    let downloadUrl = '';
    
    if (platform === 'win32') {
      downloadUrl = 'https://code.visualstudio.com/download?_exp_download=fb315fc982';
    } else if (platform === 'darwin') {
      downloadUrl = 'https://code.visualstudio.com/download';
    } else {
      downloadUrl = 'https://code.visualstudio.com/download';
    }
    
    return downloadUrl;
  }
}

module.exports = VSCodeManager;