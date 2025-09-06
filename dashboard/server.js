const express = require('express');
const cors = require('cors');
const path = require('path');
const fs = require('fs');
const chokidar = require('chokidar');
const { Server } = require('socket.io');
const http = require('http');

const app = express();
const server = http.createServer(app);
const io = new Server(server, {
  cors: {
    origin: "*",
    methods: ["GET", "POST"]
  }
});

const PORT = process.env.PORT || 3000;

// Middleware
app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, 'public')));

// Path to Arma Reforger profile directory (adjust as needed)
const ARMA_PROFILE_PATH = process.env.ARMA_PROFILE_PATH || path.join(__dirname, '..', 'data');
const STATS_FILE = path.join(ARMA_PROFILE_PATH, 'live_stats.json');

let currentStats = null;

// Load initial stats
function loadStats() {
  try {
    if (fs.existsSync(STATS_FILE)) {
      const data = fs.readFileSync(STATS_FILE, 'utf8');
      currentStats = JSON.parse(data);
      console.log('Stats loaded successfully');
    } else {
      console.log('Stats file not found, using default data');
      currentStats = {
        timestamp: Date.now(),
        server_info: {
          name: "Development Server",
          player_count: 0,
          max_players: 64
        },
        players: [],
        leaderboards: {
          top_killers: [],
          best_kd: [],
          most_active: []
        }
      };
    }
  } catch (error) {
    console.error('Error loading stats:', error);
    currentStats = null;
  }
}

// Watch for stats file changes
if (!fs.existsSync(ARMA_PROFILE_PATH)) {
  fs.mkdirSync(ARMA_PROFILE_PATH, { recursive: true });
}

const watcher = chokidar.watch(STATS_FILE, { ignoreInitial: true });
watcher.on('change', () => {
  console.log('Stats file updated, reloading...');
  loadStats();
  io.emit('stats_update', currentStats);
});

// API Routes
app.get('/api/stats', (req, res) => {
  if (!currentStats) {
    return res.status(500).json({ error: 'Stats not available' });
  }
  res.json(currentStats);
});

app.get('/api/player/:playerId', (req, res) => {
  if (!currentStats || !currentStats.players) {
    return res.status(500).json({ error: 'Stats not available' });
  }
  
  const player = currentStats.players.find(p => p.playerUID === req.params.playerId);
  if (!player) {
    return res.status(404).json({ error: 'Player not found' });
  }
  
  res.json(player);
});

app.get('/api/leaderboard/:type', (req, res) => {
  if (!currentStats || !currentStats.leaderboards) {
    return res.status(500).json({ error: 'Stats not available' });
  }
  
  const { type } = req.params;
  if (!currentStats.leaderboards[type]) {
    return res.status(404).json({ error: 'Leaderboard type not found' });
  }
  
  res.json(currentStats.leaderboards[type]);
});

app.get('/api/server-info', (req, res) => {
  if (!currentStats || !currentStats.server_info) {
    return res.status(500).json({ error: 'Server info not available' });
  }
  
  res.json(currentStats.server_info);
});

// Socket.IO for real-time updates
io.on('connection', (socket) => {
  console.log('Client connected');
  
  // Send current stats to new client
  if (currentStats) {
    socket.emit('stats_update', currentStats);
  }
  
  socket.on('disconnect', () => {
    console.log('Client disconnected');
  });
});

// Serve the dashboard
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// Load initial stats and start server
loadStats();

server.listen(PORT, () => {
  console.log(`Dashboard server running on port ${PORT}`);
  console.log(`Watching for stats file at: ${STATS_FILE}`);
});