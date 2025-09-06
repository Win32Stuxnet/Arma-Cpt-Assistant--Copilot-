class ArmaStatsDashboard {
    constructor() {
        this.socket = io();
        this.currentStats = null;
        this.charts = {};
        this.filteredPlayers = [];
        
        this.initializeEventListeners();
        this.initializeSocketEvents();
        this.loadInitialData();
    }
    
    initializeEventListeners() {
        // Tab navigation
        document.querySelectorAll('.tab-button').forEach(button => {
            button.addEventListener('click', (e) => {
                this.switchTab(e.target.getAttribute('data-tab'));
            });
        });
        
        // Player search
        const searchInput = document.getElementById('player-search');
        if (searchInput) {
            searchInput.addEventListener('input', (e) => {
                this.filterPlayers(e.target.value);
            });
        }
    }
    
    initializeSocketEvents() {
        this.socket.on('connect', () => {
            console.log('Connected to dashboard server');
            document.getElementById('status-indicator').style.background = '#4CAF50';
        });
        
        this.socket.on('disconnect', () => {
            console.log('Disconnected from dashboard server');
            document.getElementById('status-indicator').style.background = '#f44336';
        });
        
        this.socket.on('stats_update', (stats) => {
            this.currentStats = stats;
            this.updateDashboard();
        });
    }
    
    async loadInitialData() {
        try {
            const response = await fetch('/api/stats');
            if (response.ok) {
                this.currentStats = await response.json();
                this.updateDashboard();
            } else {
                console.error('Failed to load initial data');
                this.showError('Failed to load server statistics');
            }
        } catch (error) {
            console.error('Error loading initial data:', error);
            this.showError('Connection error - please check if the dashboard server is running');
        }
    }
    
    switchTab(tabName) {
        // Update buttons
        document.querySelectorAll('.tab-button').forEach(btn => btn.classList.remove('active'));
        document.querySelector(`[data-tab="${tabName}"]`).classList.add('active');
        
        // Update content
        document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));
        document.getElementById(`${tabName}-tab`).classList.add('active');
        
        // Initialize charts for analytics tab
        if (tabName === 'analytics') {
            this.initializeAnalyticsCharts();
        }
    }
    
    updateDashboard() {
        if (!this.currentStats) return;
        
        this.updateServerInfo();
        this.updateOverviewStats();
        this.updateLeaderboards();
        this.updatePlayersList();
        this.updateCharts();
        this.updateLastUpdate();
    }
    
    updateServerInfo() {
        const serverInfo = this.currentStats.server_info;
        document.getElementById('server-name').textContent = serverInfo.name || 'Unknown Server';
        document.getElementById('player-count').textContent = `${serverInfo.player_count}/${serverInfo.max_players}`;
    }
    
    updateOverviewStats() {
        const players = this.currentStats.players || [];
        
        const totalKills = players.reduce((sum, player) => sum + (player.kills || 0), 0);
        const totalDeaths = players.reduce((sum, player) => sum + (player.deaths || 0), 0);
        const totalDistance = players.reduce((sum, player) => sum + (player.distanceTraveled || 0), 0);
        
        document.getElementById('total-players').textContent = players.length;
        document.getElementById('total-kills').textContent = totalKills;
        document.getElementById('total-deaths').textContent = totalDeaths;
        document.getElementById('total-distance').textContent = (totalDistance / 1000).toFixed(1) + ' km';
    }
    
    updateLeaderboards() {
        const leaderboards = this.currentStats.leaderboards || {};
        
        this.renderLeaderboard('top-killers', leaderboards.top_killers || []);
        this.renderLeaderboard('best-kd', leaderboards.best_kd || []);
        this.renderLeaderboard('most-active', leaderboards.most_active || []);
    }
    
    renderLeaderboard(elementId, data) {
        const container = document.getElementById(elementId);
        if (!container) return;
        
        if (data.length === 0) {
            container.innerHTML = '<div class="loading">No data available</div>';
            return;
        }
        
        const html = data.map((item, index) => {
            let value;
            switch (elementId) {
                case 'top-killers':
                    value = item.kills;
                    break;
                case 'best-kd':
                    value = item.kd_ratio.toFixed(2);
                    break;
                case 'most-active':
                    value = item.playtime_hours.toFixed(1) + 'h';
                    break;
                default:
                    value = 'N/A';
            }
            
            return `
                <div class="leaderboard-item">
                    <div class="leaderboard-rank">#${index + 1}</div>
                    <div class="leaderboard-name">${item.name}</div>
                    <div class="leaderboard-value">${value}</div>
                </div>
            `;
        }).join('');
        
        container.innerHTML = html;
    }
    
    updatePlayersList() {
        const players = this.currentStats.players || [];
        this.filteredPlayers = [...players];
        this.renderPlayersList();
    }
    
    filterPlayers(searchTerm) {
        const players = this.currentStats.players || [];
        this.filteredPlayers = players.filter(player => 
            player.playerName.toLowerCase().includes(searchTerm.toLowerCase())
        );
        this.renderPlayersList();
    }
    
    renderPlayersList() {
        const container = document.getElementById('players-list');
        if (!container) return;
        
        if (this.filteredPlayers.length === 0) {
            container.innerHTML = '<div class="loading">No players found</div>';
            return;
        }
        
        const html = this.filteredPlayers.map(player => `
            <div class="player-card">
                <div class="player-name">${player.playerName}</div>
                <div class="player-stats">
                    <div class="player-stat">
                        <span class="player-stat-label">Kills:</span>
                        <span class="player-stat-value">${player.kills || 0}</span>
                    </div>
                    <div class="player-stat">
                        <span class="player-stat-label">Deaths:</span>
                        <span class="player-stat-value">${player.deaths || 0}</span>
                    </div>
                    <div class="player-stat">
                        <span class="player-stat-label">K/D Ratio:</span>
                        <span class="player-stat-value">${(player.kdRatio || 0).toFixed(2)}</span>
                    </div>
                    <div class="player-stat">
                        <span class="player-stat-label">Damage:</span>
                        <span class="player-stat-value">${Math.round(player.damageDealt || 0)}</span>
                    </div>
                    <div class="player-stat">
                        <span class="player-stat-label">Distance:</span>
                        <span class="player-stat-value">${((player.distanceTraveled || 0) / 1000).toFixed(1)} km</span>
                    </div>
                    <div class="player-stat">
                        <span class="player-stat-label">Playtime:</span>
                        <span class="player-stat-value">${((player.totalPlayTime || 0) / 3600).toFixed(1)}h</span>
                    </div>
                </div>
            </div>
        `).join('');
        
        container.innerHTML = html;
    }
    
    updateCharts() {
        this.updateKDChart();
        this.updateWeaponsChart();
    }
    
    updateKDChart() {
        const ctx = document.getElementById('kd-chart');
        if (!ctx) return;
        
        const players = this.currentStats.players || [];
        const kdRatios = players.map(p => p.kdRatio || 0);
        
        // Create K/D ratio distribution
        const ranges = ['0-0.5', '0.5-1.0', '1.0-1.5', '1.5-2.0', '2.0+'];
        const distribution = [0, 0, 0, 0, 0];
        
        kdRatios.forEach(kd => {
            if (kd < 0.5) distribution[0]++;
            else if (kd < 1.0) distribution[1]++;
            else if (kd < 1.5) distribution[2]++;
            else if (kd < 2.0) distribution[3]++;
            else distribution[4]++;
        });
        
        if (this.charts.kdChart) {
            this.charts.kdChart.destroy();
        }
        
        this.charts.kdChart = new Chart(ctx, {
            type: 'bar',
            data: {
                labels: ranges,
                datasets: [{
                    label: 'Players',
                    data: distribution,
                    backgroundColor: 'rgba(102, 126, 234, 0.6)',
                    borderColor: 'rgba(102, 126, 234, 1)',
                    borderWidth: 2
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    legend: {
                        labels: { color: '#ffffff' }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        ticks: { color: '#ffffff' },
                        grid: { color: 'rgba(255, 255, 255, 0.1)' }
                    },
                    x: {
                        ticks: { color: '#ffffff' },
                        grid: { color: 'rgba(255, 255, 255, 0.1)' }
                    }
                }
            }
        });
    }
    
    updateWeaponsChart() {
        const ctx = document.getElementById('weapons-chart');
        if (!ctx) return;
        
        const players = this.currentStats.players || [];
        const weaponUsage = {};
        
        players.forEach(player => {
            if (player.weaponUsage) {
                Object.entries(player.weaponUsage).forEach(([weapon, usage]) => {
                    weaponUsage[weapon] = (weaponUsage[weapon] || 0) + usage;
                });
            }
        });
        
        const sortedWeapons = Object.entries(weaponUsage)
            .sort((a, b) => b[1] - a[1])
            .slice(0, 10);
        
        if (this.charts.weaponsChart) {
            this.charts.weaponsChart.destroy();
        }
        
        this.charts.weaponsChart = new Chart(ctx, {
            type: 'doughnut',
            data: {
                labels: sortedWeapons.map(w => w[0]),
                datasets: [{
                    data: sortedWeapons.map(w => w[1]),
                    backgroundColor: [
                        'rgba(255, 99, 132, 0.6)',
                        'rgba(54, 162, 235, 0.6)',
                        'rgba(255, 205, 86, 0.6)',
                        'rgba(75, 192, 192, 0.6)',
                        'rgba(153, 102, 255, 0.6)',
                        'rgba(255, 159, 64, 0.6)',
                        'rgba(199, 199, 199, 0.6)',
                        'rgba(83, 102, 255, 0.6)',
                        'rgba(255, 99, 255, 0.6)',
                        'rgba(99, 255, 132, 0.6)'
                    ]
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    legend: {
                        labels: { color: '#ffffff' }
                    }
                }
            }
        });
    }
    
    initializeAnalyticsCharts() {
        // Initialize analytics charts when tab is opened
        setTimeout(() => {
            this.updateActivityChart();
            this.updateDamageChart();
        }, 100);
    }
    
    updateActivityChart() {
        const ctx = document.getElementById('activity-chart');
        if (!ctx) return;
        
        // Mock data for activity over time - in real implementation,
        // this would come from historical event data
        const mockData = [
            { time: '00:00', players: 5 },
            { time: '04:00', players: 2 },
            { time: '08:00', players: 8 },
            { time: '12:00', players: 15 },
            { time: '16:00', players: 22 },
            { time: '20:00', players: 18 },
            { time: '24:00', players: 12 }
        ];
        
        if (this.charts.activityChart) {
            this.charts.activityChart.destroy();
        }
        
        this.charts.activityChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: mockData.map(d => d.time),
                datasets: [{
                    label: 'Active Players',
                    data: mockData.map(d => d.players),
                    borderColor: 'rgba(118, 75, 162, 1)',
                    backgroundColor: 'rgba(118, 75, 162, 0.1)',
                    tension: 0.4,
                    fill: true
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    legend: {
                        labels: { color: '#ffffff' }
                    }
                },
                scales: {
                    y: {
                        beginAtZero: true,
                        ticks: { color: '#ffffff' },
                        grid: { color: 'rgba(255, 255, 255, 0.1)' }
                    },
                    x: {
                        ticks: { color: '#ffffff' },
                        grid: { color: 'rgba(255, 255, 255, 0.1)' }
                    }
                }
            }
        });
    }
    
    updateDamageChart() {
        const ctx = document.getElementById('damage-chart');
        if (!ctx) return;
        
        const players = this.currentStats.players || [];
        const damageData = players.map(player => ({
            x: player.damageDealt || 0,
            y: player.damageTaken || 0,
            label: player.playerName
        }));
        
        if (this.charts.damageChart) {
            this.charts.damageChart.destroy();
        }
        
        this.charts.damageChart = new Chart(ctx, {
            type: 'scatter',
            data: {
                datasets: [{
                    label: 'Players',
                    data: damageData,
                    backgroundColor: 'rgba(102, 126, 234, 0.6)',
                    borderColor: 'rgba(102, 126, 234, 1)'
                }]
            },
            options: {
                responsive: true,
                plugins: {
                    legend: {
                        labels: { color: '#ffffff' }
                    }
                },
                scales: {
                    x: {
                        title: {
                            display: true,
                            text: 'Damage Dealt',
                            color: '#ffffff'
                        },
                        ticks: { color: '#ffffff' },
                        grid: { color: 'rgba(255, 255, 255, 0.1)' }
                    },
                    y: {
                        title: {
                            display: true,
                            text: 'Damage Taken',
                            color: '#ffffff'
                        },
                        ticks: { color: '#ffffff' },
                        grid: { color: 'rgba(255, 255, 255, 0.1)' }
                    }
                }
            }
        });
    }
    
    updateLastUpdate() {
        if (this.currentStats && this.currentStats.timestamp) {
            const date = new Date(this.currentStats.timestamp * 1000);
            document.getElementById('last-update').textContent = date.toLocaleString();
        }
    }
    
    showError(message) {
        // Create a simple error notification
        const errorDiv = document.createElement('div');
        errorDiv.style.cssText = `
            position: fixed;
            top: 20px;
            right: 20px;
            background: rgba(244, 67, 54, 0.9);
            color: white;
            padding: 1rem 2rem;
            border-radius: 8px;
            z-index: 1000;
            max-width: 400px;
        `;
        errorDiv.textContent = message;
        
        document.body.appendChild(errorDiv);
        
        setTimeout(() => {
            document.body.removeChild(errorDiv);
        }, 5000);
    }
}

// Initialize dashboard when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    new ArmaStatsDashboard();
});