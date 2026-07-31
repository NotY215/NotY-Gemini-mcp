const CryptoJS = require('crypto-js');
const crypto = require('crypto');
const os = require('os');
const path = require('path');
const fs = require('fs');

class Encryption {
  constructor() {
    // Generate a unique key based on system information
    this.key = this.generateKey();
  }

  generateKey() {
    // Use system information to generate a unique key
    const systemInfo = [
      os.hostname(),
      os.userInfo().username,
      os.totalmem().toString(),
      os.cpus().map(cpu => cpu.model).join('')
    ].join('|');
    
    // Create a hash of the system info
    return crypto.createHash('sha256').update(systemInfo).digest('hex');
  }

  encrypt(text) {
    if (!text) return null;
    try {
      const encrypted = CryptoJS.AES.encrypt(text, this.key).toString();
      return encrypted;
    } catch (error) {
      console.error('Encryption error:', error);
      return null;
    }
  }

  decrypt(encryptedText) {
    if (!encryptedText) return null;
    try {
      const decrypted = CryptoJS.AES.decrypt(encryptedText, this.key);
      return decrypted.toString(CryptoJS.enc.Utf8);
    } catch (error) {
      console.error('Decryption error:', error);
      return null;
    }
  }

  // Store encrypted data in a secure location
  storeSecureData(key, data) {
    const appData = process.env.APPDATA || (process.platform === 'darwin' ? 
      path.join(os.homedir(), 'Library', 'Application Support') : 
      path.join(os.homedir(), '.config'));
    
    const geminiDir = path.join(appData, 'gemini-mcp');
    if (!fs.existsSync(geminiDir)) {
      fs.mkdirSync(geminiDir, { recursive: true });
    }
    
    const filePath = path.join(geminiDir, 'key.db');
    const encrypted = this.encrypt(JSON.stringify(data));
    fs.writeFileSync(filePath, encrypted);
  }

  getSecureData(key) {
    const appData = process.env.APPDATA || (process.platform === 'darwin' ? 
      path.join(os.homedir(), 'Library', 'Application Support') : 
      path.join(os.homedir(), '.config'));
    
    const filePath = path.join(appData, 'gemini-mcp', 'key.db');
    if (!fs.existsSync(filePath)) {
      return null;
    }
    
    const encrypted = fs.readFileSync(filePath, 'utf8');
    const decrypted = this.decrypt(encrypted);
    if (decrypted) {
      return JSON.parse(decrypted);
    }
    return null;
  }
}

module.exports = Encryption;