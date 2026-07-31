const { GoogleGenerativeAI, HarmCategory, HarmBlockThreshold } = require('@google/generative-ai');

class GeminiService {
  constructor(apiKey) {
    this.apiKey = apiKey;
    this.genAI = new GoogleGenerativeAI(apiKey);
    this.model = null;
    this.chat = null;
    this.history = [];
  }

  async initialize() {
    try {
      this.model = this.genAI.getGenerativeModel({ 
        model: 'gemini-pro',
        safetySettings: [
          {
            category: HarmCategory.HARM_CATEGORY_HARASSMENT,
            threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE,
          },
          {
            category: HarmCategory.HARM_CATEGORY_HATE_SPEECH,
            threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE,
          },
          {
            category: HarmCategory.HARM_CATEGORY_SEXUALLY_EXPLICIT,
            threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE,
          },
          {
            category: HarmCategory.HARM_CATEGORY_DANGEROUS_CONTENT,
            threshold: HarmBlockThreshold.BLOCK_MEDIUM_AND_ABOVE,
          },
        ]
      });
      
      this.chat = this.model.startChat({
        history: this.history,
        generationConfig: {
          maxOutputTokens: 2048,
          temperature: 0.7,
          topP: 0.8,
          topK: 40,
        },
      });
      
      return true;
    } catch (error) {
      console.error('Error initializing Gemini:', error);
      return false;
    }
  }

  async verifyKey() {
    try {
      await this.initialize();
      return true;
    } catch (error) {
      console.error('API key verification failed:', error);
      return false;
    }
  }

  async sendMessage(message, context = null) {
    try {
      if (!this.model || !this.chat) {
        await this.initialize();
      }
      
      let fullMessage = message;
      
      // Add context if provided
      if (context) {
        fullMessage = `Context: ${context}\n\nUser: ${message}`;
      }
      
      const result = await this.chat.sendMessage(fullMessage);
      const response = await result.response;
      const text = response.text();
      
      // Add to history
      this.history.push({ role: 'user', parts: [{ text: fullMessage }] });
      this.history.push({ role: 'model', parts: [{ text: text }] });
      
      // Keep history limited
      if (this.history.length > 20) {
        this.history = this.history.slice(-20);
      }
      
      return text;
    } catch (error) {
      console.error('Error sending message to Gemini:', error);
      throw error;
    }
  }

  async analyzeCode(code, question) {
    const prompt = `
You are a coding assistant integrated with VS Code. You have access to the current project files.

Current Code:
\`\`\`
${code}
\`\`\`

Question: ${question}

Please provide a comprehensive analysis, including:
1. Code quality assessment
2. Potential bugs or issues
3. Performance improvements
4. Best practices
5. Alternative approaches (if applicable)
6. Direct code fixes if needed

Make your response actionable and clear. If you're suggesting code changes, show the before and after code.
`;
    
    return await this.sendMessage(prompt);
  }

  async fixErrors(errorLog, code) {
    const prompt = `
You are a coding assistant helping to fix errors in a VS Code project.

Error Log:
${errorLog}

Current Code:
\`\`\`
${code || 'No code provided'}
\`\`\`

Please analyze the error and provide:
1. Root cause analysis
2. Step-by-step fix instructions
3. The corrected code (if applicable)
4. Prevention tips for the future

Be specific and provide actionable solutions.
`;
    
    return await this.sendMessage(prompt);
  }

  clearHistory() {
    this.history = [];
    if (this.chat) {
      this.chat = this.model.startChat({
        history: [],
        generationConfig: {
          maxOutputTokens: 2048,
          temperature: 0.7,
          topP: 0.8,
          topK: 40,
        },
      });
    }
  }
}

module.exports = GeminiService;