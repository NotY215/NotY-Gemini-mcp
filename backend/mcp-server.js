const express = require('express');
const WebSocket = require('ws');
const http = require('http');
const { spawn } = require('child_process');
const fs = require('fs-extra');
const path = require('path');
const os = require('os');

class MCPServer {
  constructor(geminiService, vscodePath) {
    this.geminiService = geminiService;
    this.vscodePath = vscodePath;
    this.app = express();
    this.server = null;
    this.wsServer = null;
    this.isRunning = false;
    this.terminalSessions = new Map();
    this.currentProjectPath = null;
    
    this.setupMiddleware();
    this.setupRoutes();
    this.setupWebSocket();
  }

  setupMiddleware() {
    this.app.use(express.json());
    this.app.use(express.urlencoded({ extended: true }));
    
    // CORS for development
    this.app.use((req, res, next) => {
      res.header('Access-Control-Allow-Origin', '*');
      res.header('Access-Control-Allow-Headers', 'Origin, X-Requested-With, Content-Type, Accept');
      res.header('Access-Control-Allow-Methods', 'GET, POST, PUT, DELETE, OPTIONS');
      next();
    });
  }

  setupRoutes() {
    // Health check
    this.app.get('/health', (req, res) => {
      res.json({ status: 'running', timestamp: new Date().toISOString() });
    });

    // Send message to Gemini
    this.app.post('/api/chat', async (req, res) => {
      try {
        const { message, context } = req.body;
        const response = await this.geminiService.sendMessage(message, context);
        res.json({ success: true, response });
      } catch (error) {
        res.status(500).json({ success: false, error: error.message });
      }
    });

    // Analyze code
    this.app.post('/api/analyze', async (req, res) => {
      try {
        const { code, question } = req.body;
        const response = await this.geminiService.analyzeCode(code, question);
        res.json({ success: true, response });
      } catch (error) {
        res.status(500).json({ success: false, error: error.message });
      }
    });

    // Fix errors
    this.app.post('/api/fix-errors', async (req, res) => {
      try {
        const { errorLog, code } = req.body;
        const response = await this.geminiService.fixErrors(errorLog, code);
        res.json({ success: true, response });
      } catch (error) {
        res.status(500).json({ success: false, error: error.message });
      }
    });

    // Get project context
    this.app.get('/api/project-context', async (req, res) => {
      try {
        const projectPath = this.currentProjectPath || process.cwd();
        const files = await this.getProjectFiles(projectPath);
        res.json({ success: true, projectPath, files });
      } catch (error) {
        res.status(500).json({ success: false, error: error.message });
      }
    });

    // Set project path
    this.app.post('/api/set-project', (req, res) => {
      try {
        const { projectPath } = req.body;
        if (fs.existsSync(projectPath)) {
          this.currentProjectPath = projectPath;
          res.json({ success: true, projectPath });
        } else {
          res.status(400).json({ success: false, error: 'Project path does not exist' });
        }
      } catch (error) {
        res.status(500).json({ success: false, error: error.message });
      }
    });
  }

  setupWebSocket() {
    this.server = http.createServer(this.app);
    this.wsServer = new WebSocket.Server({ server: this.server });
    
    this.wsServer.on('connection', (ws, req) => {
      console.log('WebSocket client connected');
      
      ws.on('message', async (message) => {
        try {
          const data = JSON.parse(message);
          
          switch (data.type) {
            case 'chat':
              const response = await this.geminiService.sendMessage(data.message, data.context);
              ws.send(JSON.stringify({ type: 'chat-response', response }));
              break;
              
            case 'terminal':
              this.handleTerminalCommand(ws, data);
              break;
              
            case 'analyze':
              const analysis = await this.geminiService.analyzeCode(data.code, data.question);
              ws.send(JSON.stringify({ type: 'analyze-response', analysis }));
              break;
              
            case 'fix-errors':
              const fix = await this.geminiService.fixErrors(data.errorLog, data.code);
              ws.send(JSON.stringify({ type: 'fix-response', fix }));
              break;
              
            default:
              ws.send(JSON.stringify({ type: 'error', message: 'Unknown message type' }));
          }
        } catch (error) {
          ws.send(JSON.stringify({ type: 'error', message: error.message }));
        }
      });
      
      ws.on('close', () => {
        console.log('WebSocket client disconnected');
      });
    });
  }

  handleTerminalCommand(ws, data) {
    const { command, sessionId } = data;
    
    if (!this.terminalSessions.has(sessionId)) {
      // Create new terminal session
      const terminal = spawn(this.getShell(), [], {
        cwd: this.currentProjectPath || process.cwd(),
        env: process.env
      });
      
      terminal.stdout.on('data', (data) => {
        ws.send(JSON.stringify({ 
          type: 'terminal-output', 
          sessionId, 
          output: data.toString() 
        }));
      });
      
      terminal.stderr.on('data', (data) => {
        ws.send(JSON.stringify({ 
          type: 'terminal-output', 
          sessionId, 
          output: data.toString(),
          isError: true 
        }));
      });
      
      terminal.on('close', () => {
        this.terminalSessions.delete(sessionId);
        ws.send(JSON.stringify({ 
          type: 'terminal-close', 
          sessionId 
        }));
      });
      
      this.terminalSessions.set(sessionId, terminal);
    }
    
    const terminal = this.terminalSessions.get(sessionId);
    if (terminal && terminal.stdin) {
      terminal.stdin.write(command + '\n');
    }
  }

  getShell() {
    const platform = os.platform();
    if (platform === 'win32') {
      return 'cmd.exe';
    } else if (platform === 'darwin') {
      return '/bin/zsh';
    } else {
      return '/bin/bash';
    }
  }

  async getProjectFiles(projectPath, maxFiles = 50) {
    const files = [];
    
    try {
      const entries = await fs.readdir(projectPath);
      
      for (const entry of entries) {
        if (files.length >= maxFiles) break;
        
        const fullPath = path.join(projectPath, entry);
        const stats = await fs.stat(fullPath);
        
        // Skip node_modules, .git, etc.
        if (entry === 'node_modules' || entry === '.git' || entry === 'dist' || entry === 'build') {
          continue;
        }
        
        if (stats.isFile()) {
          const ext = path.extname(entry);
          const relevantExtensions = ['.js', '.jsx', '.ts', '.tsx', '.py', '.java', '.c', '.cpp', '.html', '.css', '.json', '.md'];
          
          if (relevantExtensions.includes(ext)) {
            try {
              const content = await fs.readFile(fullPath, 'utf8');
              files.push({
                name: entry,
                path: fullPath,
                content: content.length > 10000 ? content.substring(0, 10000) + '...' : content,
                size: stats.size,
                ext: ext
              });
            } catch (error) {
              // Skip files that can't be read
            }
          }
        } else if (stats.isDirectory()) {
          const subFiles = await this.getProjectFiles(fullPath, maxFiles - files.length);
          files.push(...subFiles);
        }
      }
    } catch (error) {
      console.error('Error reading project files:', error);
    }
    
    return files;
  }

  async start() {
    return new Promise((resolve, reject) => {
      const port = 31415; // Gemini MCP port
      
      this.server.listen(port, '127.0.0.1', () => {
        this.isRunning = true;
        console.log(`MCP Server running on port ${port}`);
        resolve();
      });
      
      this.server.on('error', (error) => {
        this.isRunning = false;
        reject(error);
      });
    });
  }

  async stop() {
    return new Promise((resolve) => {
      if (this.wsServer) {
        this.wsServer.close();
      }
      
      if (this.server) {
        this.server.close(() => {
          this.isRunning = false;
          resolve();
        });
      } else {
        resolve();
      }
      
      // Close all terminal sessions
      for (const [sessionId, terminal] of this.terminalSessions) {
        if (terminal && terminal.kill) {
          terminal.kill();
        }
      }
      this.terminalSessions.clear();
    });
  }
}

module.exports = MCPServer;