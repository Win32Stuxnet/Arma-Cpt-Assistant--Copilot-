const express = require('express');
const cors = require('cors');
const axios = require('axios');
const fs = require('fs');
const path = require('path');
const chokidar = require('chokidar');
const winston = require('winston');
require('dotenv').config();

const app = express();
const PORT = process.env.PORT || 3001;

// Configure logging
const logger = winston.createLogger({
  level: 'info',
  format: winston.format.combine(
    winston.format.timestamp(),
    winston.format.json()
  ),
  transports: [
    new winston.transports.File({ filename: 'bridge-error.log', level: 'error' }),
    new winston.transports.File({ filename: 'bridge-combined.log' }),
    new winston.transports.Console({
      format: winston.format.simple()
    })
  ]
});

// Middleware
app.use(cors());
app.use(express.json({ limit: '50mb' }));

// Configuration
const config = {
  armaProfilePath: process.env.ARMA_PROFILE_PATH || path.join(__dirname, '..', 'data'),
  aiServices: {
    claude: {
      endpoint: 'https://api.anthropic.com/v1/messages',
      apiKey: process.env.CLAUDE_API_KEY,
      headers: {
        'Content-Type': 'application/json',
        'x-api-key': process.env.CLAUDE_API_KEY,
        'anthropic-version': '2023-06-01'
      }
    },
    openai: {
      endpoint: 'https://api.openai.com/v1/chat/completions',
      apiKey: process.env.OPENAI_API_KEY,
      headers: {
        'Content-Type': 'application/json',
        'Authorization': `Bearer ${process.env.OPENAI_API_KEY}`
      }
    },
    ollama: {
      endpoint: 'http://localhost:11434/api/generate',
      headers: {
        'Content-Type': 'application/json'
      }
    }
  },
  rateLimits: {
    claude: { requests: 100, window: 60000 }, // 100 requests per minute
    openai: { requests: 60, window: 60000 },  // 60 requests per minute
    ollama: { requests: 1000, window: 60000 } // 1000 requests per minute
  }
};

// Rate limiting tracking
const rateLimitTracking = {};

// File-based communication system
const REQUEST_FILE = path.join(config.armaProfilePath, 'ai_request.json');
const RESPONSE_FILE = path.join(config.armaProfilePath, 'ai_response.json');

// Ensure directories exist
if (!fs.existsSync(config.armaProfilePath)) {
  fs.mkdirSync(config.armaProfilePath, { recursive: true });
}

// API Routes

// Health check
app.get('/health', (req, res) => {
  res.json({ 
    status: 'ok', 
    timestamp: new Date().toISOString(),
    services: {
      claude: !!config.aiServices.claude.apiKey,
      openai: !!config.aiServices.openai.apiKey,
      ollama: true
    }
  });
});

// Main AI request endpoint
app.post('/api/ai-request', async (req, res) => {
  try {
    const { service, prompt, model, settings } = req.body;
    
    if (!prompt) {
      return res.status(400).json({ error: 'Prompt is required' });
    }
    
    // Check rate limits
    if (!checkRateLimit(service)) {
      return res.status(429).json({ error: 'Rate limit exceeded' });
    }
    
    logger.info(`Processing AI request for service: ${service}`);
    
    const response = await processAIRequest(service, prompt, model, settings);
    
    res.json({ 
      success: true, 
      response: response,
      service: service,
      timestamp: new Date().toISOString()
    });
    
  } catch (error) {
    logger.error('AI request failed', error);
    res.status(500).json({ 
      error: 'AI request failed', 
      details: error.message 
    });
  }
});

// File-based communication endpoint
app.post('/api/process-file-request', async (req, res) => {
  try {
    if (!fs.existsSync(REQUEST_FILE)) {
      return res.status(404).json({ error: 'No request file found' });
    }
    
    const requestData = JSON.parse(fs.readFileSync(REQUEST_FILE, 'utf8'));
    const response = await processAIRequest(
      requestData.service, 
      requestData.prompt, 
      requestData.model, 
      requestData.settings
    );
    
    // Write response to file
    fs.writeFileSync(RESPONSE_FILE, JSON.stringify({
      response: response,
      timestamp: new Date().toISOString(),
      success: true
    }, null, 2));
    
    // Clean up request file
    fs.unlinkSync(REQUEST_FILE);
    
    res.json({ success: true, message: 'Request processed and response written to file' });
    
  } catch (error) {
    logger.error('File-based request failed', error);
    
    // Write error response to file
    fs.writeFileSync(RESPONSE_FILE, JSON.stringify({
      error: error.message,
      timestamp: new Date().toISOString(),
      success: false
    }, null, 2));
    
    res.status(500).json({ error: 'File-based request failed', details: error.message });
  }
});

// Configuration endpoint
app.get('/api/config', (req, res) => {
  res.json({
    services: Object.keys(config.aiServices),
    rateLimits: config.rateLimits,
    profilePath: config.armaProfilePath
  });
});

// Process AI request based on service
async function processAIRequest(service, prompt, model = null, settings = {}) {
  const serviceConfig = config.aiServices[service];
  
  if (!serviceConfig) {
    throw new Error(`Unsupported AI service: ${service}`);
  }
  
  let payload, response;
  
  switch (service) {
    case 'claude':
      payload = {
        model: model || 'claude-3-sonnet-20240229',
        max_tokens: settings.maxTokens || 4000,
        messages: [{ role: 'user', content: prompt }]
      };
      break;
      
    case 'openai':
      payload = {
        model: model || 'gpt-3.5-turbo',
        messages: [{ role: 'user', content: prompt }],
        max_tokens: settings.maxTokens || 4000,
        temperature: settings.temperature || 0.7
      };
      break;
      
    case 'ollama':
      payload = {
        model: model || 'codellama',
        prompt: prompt,
        stream: false
      };
      break;
      
    default:
      throw new Error(`Unsupported service: ${service}`);
  }
  
  try {
    const axiosResponse = await axios.post(serviceConfig.endpoint, payload, {
      headers: serviceConfig.headers,
      timeout: settings.timeout || 60000
    });
    
    // Extract response text based on service
    let responseText;
    
    switch (service) {
      case 'claude':
        responseText = axiosResponse.data.content[0].text;
        break;
      case 'openai':
        responseText = axiosResponse.data.choices[0].message.content;
        break;
      case 'ollama':
        responseText = axiosResponse.data.response;
        break;
    }
    
    logger.info(`AI request completed successfully for service: ${service}`);
    return responseText;
    
  } catch (error) {
    logger.error(`AI service error for ${service}:`, error.response?.data || error.message);
    throw new Error(`AI service error: ${error.response?.data?.error?.message || error.message}`);
  }
}

// Rate limiting
function checkRateLimit(service) {
  const limits = config.rateLimits[service];
  if (!limits) return true;
  
  const now = Date.now();
  const serviceTracking = rateLimitTracking[service] || { requests: [], count: 0 };
  
  // Remove expired requests
  serviceTracking.requests = serviceTracking.requests.filter(
    timestamp => now - timestamp < limits.window
  );
  
  if (serviceTracking.requests.length >= limits.requests) {
    return false;
  }
  
  serviceTracking.requests.push(now);
  rateLimitTracking[service] = serviceTracking;
  
  return true;
}

// File watcher for Arma integration
const watcher = chokidar.watch(REQUEST_FILE, { ignoreInitial: true });

watcher.on('add', async () => {
  logger.info('New request file detected, processing...');
  
  try {
    // Wait a moment for file write to complete
    await new Promise(resolve => setTimeout(resolve, 100));
    
    const requestData = JSON.parse(fs.readFileSync(REQUEST_FILE, 'utf8'));
    const response = await processAIRequest(
      requestData.service, 
      requestData.prompt, 
      requestData.model, 
      requestData.settings
    );
    
    // Write response
    fs.writeFileSync(RESPONSE_FILE, JSON.stringify({
      response: response,
      timestamp: new Date().toISOString(),
      success: true
    }, null, 2));
    
    // Clean up request file
    fs.unlinkSync(REQUEST_FILE);
    
    logger.info('File-based request processed successfully');
    
  } catch (error) {
    logger.error('File-based request processing failed:', error);
    
    // Write error response
    fs.writeFileSync(RESPONSE_FILE, JSON.stringify({
      error: error.message,
      timestamp: new Date().toISOString(),
      success: false
    }, null, 2));
  }
});

// Error handling middleware
app.use((error, req, res, next) => {
  logger.error('Unhandled error:', error);
  res.status(500).json({ error: 'Internal server error' });
});

// Start server
app.listen(PORT, () => {
  logger.info(`AI Bridge Service started on port ${PORT}`);
  logger.info(`Watching for requests in: ${config.armaProfilePath}`);
  logger.info('Available services:', Object.keys(config.aiServices));
});

// Graceful shutdown
process.on('SIGINT', () => {
  logger.info('Shutting down AI Bridge Service...');
  watcher.close();
  process.exit(0);
});