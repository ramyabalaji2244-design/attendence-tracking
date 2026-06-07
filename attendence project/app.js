// Global Variables
let map;
let studentMarkers = [];
let attendanceChart;
const API_BASE_URL = 'http://localhost:5000/api';

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    initializeMap();
    loadDashboardData();
    setupEventListeners();
    startLiveUpdates();
    initializeAttendanceChart();
});

// ==================== Navigation Functions ====================
function showTab(tabName) {
    // Hide all tabs
    const tabs = document.querySelectorAll('.tab-content');
    tabs.forEach(tab => tab.classList.remove('active'));

    // Remove active class from all buttons
    const buttons = document.querySelectorAll('.nav-btn');
    buttons.forEach(btn => btn.classList.remove('active'));

    // Show selected tab
    const selectedTab = document.getElementById(tabName);
    if (selectedTab) {
        selectedTab.classList.add('active');
        event.target.classList.add('active');
    }

    // Update map when tracking tab is opened
    if (tabName === 'tracking') {
        setTimeout(() => {
            if (map) map.invalidateSize();
        }, 100);
    }
}

// ==================== Dashboard Functions ====================
function loadDashboardData() {
    // Simulated data - Replace with actual API calls
    const studentData = {
        total: 5,
        present: 5,
        attendance: 92,
        activeDevices: 5
    };

    document.getElementById('studentCount').textContent = studentData.total;
    document.getElementById('attendancePercent').textContent = studentData.attendance + '%';
    document.getElementById('activeDevices').textContent = studentData.activeDevices;
    document.getElementById('lastUpdate').textContent = '2 min ago';
}

function initializeAttendanceChart() {
    const ctx = document.getElementById('attendanceChart').getContext('2d');
    
    attendanceChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'],
            datasets: [{
                label: 'Attendance %',
                data: [88, 90, 92, 95, 93, 89, 92],
                borderColor: '#667eea',
                backgroundColor: 'rgba(102, 126, 234, 0.1)',
                borderWidth: 2,
                tension: 0.4,
                fill: true,
                pointRadius: 5,
                pointBackgroundColor: '#667eea',
                pointHoverRadius: 7
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    display: true,
                    position: 'top'
                }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    max: 100,
                    ticks: {
                        callback: function(value) {
                            return value + '%';
                        }
                    }
                }
            }
        }
    });
}

// ==================== Map Functions ====================
function initializeMap() {
    // Initialize Leaflet map
    map = L.map('map').setView([12.9716, 77.5946], 13);

    // Add OpenStreetMap tile layer
    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
        attribution: '© OpenStreetMap contributors',
        maxZoom: 19
    }).addTo(map);

    // Add initial markers
    addStudentMarkers();
}

function addStudentMarkers() {
    const students = [
        { id: 1, name: 'Ravi Kumar', lat: 12.9716, lng: 77.5946, status: 'in-transit' },
        { id: 2, name: 'Emma Wilson', lat: 12.9352, lng: 77.6245, status: 'at-school' },
        { id: 3, name: 'Alex Johnson', lat: 12.9550, lng: 77.5800, status: 'in-transit' },
        { id: 4, name: 'Sophia Martinez', lat: 12.9250, lng: 77.6100, status: 'at-home' },
        { id: 5, name: 'Oliver Brown', lat: 12.9450, lng: 77.5950, status: 'in-transit' }
    ];

    students.forEach(student => {
        const marker = L.marker([student.lat, student.lng])
            .bindPopup(`<b>${student.name}</b><br/>Status: ${student.status}`)
            .addTo(map);
        
        studentMarkers.push({ marker, student });
    });
}

function centerMap(lat, lng) {
    map.flyTo([lat, lng], 16);
}

function updateStudentLocations() {
    // Simulated location update
    const locations = [
        { id: 1, lat: 12.9716, lng: 77.5946, speed: 25 },
        { id: 2, lat: 12.9352, lng: 77.6245, speed: 0 }
    ];

    locations.forEach(location => {
        // Update marker position
        // This would typically come from real GPS data
    });
}

// ==================== Attendance Functions ====================
function updateAttendanceTable() {
    const date = document.getElementById('attendanceDate').value;
    const student = document.getElementById('studentFilter').value;

    // Filter and update attendance records
    console.log(`Updating attendance for ${date} - ${student}`);

    // This would make an API call to fetch filtered data
    // Example: fetchAttendanceData(date, student)
}

// ==================== Alert Functions ====================
function alertAction(action) {
    if (action === 'acknowledge') {
        console.log('Alert acknowledged');
        // Send acknowledgment to backend
    } else if (action === 'investigate') {
        console.log('Investigating alert');
        // Take investigation action
    }
}

// ==================== Settings Functions ====================
function saveSettings() {
    const settings = {
        notifications: {
            attendance: document.querySelector('input[type="checkbox"]').checked,
            location: document.querySelectorAll('input[type="checkbox"]')[1]?.checked,
            unusual: document.querySelectorAll('input[type="checkbox"]')[2]?.checked
        },
        geofence: {
            radius: document.querySelector('input[type="number"]')?.value,
            alertTime: document.querySelector('select')?.value
        }
    };

    console.log('Settings saved:', settings);
    alert('Settings have been saved successfully!');
    
    // Send to backend API
    // updateSettings(settings)
}

// ==================== Utility Functions ====================
function startLiveUpdates() {
    // Update data every 30 seconds
    setInterval(() => {
        updateStudentLocations();
        loadDashboardData();
    }, 30000);
}

function setupEventListeners() {
    // Set today's date in attendance filter
    const today = new Date().toISOString().split('T')[0];
    const dateInput = document.getElementById('attendanceDate');
    if (dateInput) {
        dateInput.value = today;
    }
}

function logout() {
    if (confirm('Are you sure you want to logout?')) {
        // Clear session
        sessionStorage.clear();
        // Redirect to login page
        window.location.href = 'login.html';
    }
}

// ==================== API Communication Functions ====================
async function fetchData(endpoint) {
    try {
        const response = await fetch(`${API_BASE_URL}${endpoint}`);
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error('Fetch error:', error);
        return null;
    }
}

async function sendData(endpoint, data, method = 'POST') {
    try {
        const response = await fetch(`${API_BASE_URL}${endpoint}`, {
            method: method,
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data)
        });
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        return await response.json();
    } catch (error) {
        console.error('Send error:', error);
        return null;
    }
}

// ==================== Real-time WebSocket Connection ====================
let ws;

function connectWebSocket() {
    ws = new WebSocket('ws://localhost:5000/ws/tracking');

    ws.onopen = function(event) {
        console.log('WebSocket connection established');
    };

    ws.onmessage = function(event) {
        const data = JSON.parse(event.data);
        
        // Update location data in real-time
        if (data.type === 'location_update') {
            updateMarkerLocation(data.studentId, data.lat, data.lng);
        }
        
        // Handle new alerts
        if (data.type === 'alert') {
            displayAlert(data);
        }
        
        // Handle attendance events
        if (data.type === 'attendance') {
            updateActivityList(data);
        }
    };

    ws.onerror = function(error) {
        console.error('WebSocket error:', error);
    };

    ws.onclose = function(event) {
        console.log('WebSocket connection closed');
        // Attempt to reconnect after 5 seconds
        setTimeout(connectWebSocket, 5000);
    };
}

function updateMarkerLocation(studentId, lat, lng) {
    const markerData = studentMarkers.find(m => m.student.id === studentId);
    if (markerData) {
        markerData.marker.setLatLng([lat, lng]);
        markerData.student.lat = lat;
        markerData.student.lng = lng;
    }
}

function displayAlert(alertData) {
    console.log('New alert:', alertData);
    // Add alert to the alerts container
}

function updateActivityList(activityData) {
    const activityList = document.getElementById('activityList');
    if (activityList) {
        const newActivity = document.createElement('div');
        newActivity.className = 'activity-item';
        newActivity.innerHTML = `
            <span class="time">${new Date().toLocaleTimeString()}</span>
            <span class="message">${activityData.message}</span>
            <span class="status success">✓</span>
        `;
        activityList.insertBefore(newActivity, activityList.firstChild);
    }
}

// Start WebSocket connection on page load
window.addEventListener('load', connectWebSocket);

// ==================== Data Export Functions ====================
function exportAttendanceReport() {
    // Export attendance data as CSV
    const csv = generateCSV();
    downloadFile(csv, 'attendance-report.csv', 'text/csv');
}

function generateCSV() {
    const table = document.querySelector('.attendance-table');
    let csv = '';

    // Add header
    const headers = table.querySelectorAll('th');
    headers.forEach((header, index) => {
        csv += header.textContent + (index < headers.length - 1 ? ',' : '\n');
    });

    // Add rows
    const rows = table.querySelectorAll('tbody tr');
    rows.forEach(row => {
        const cells = row.querySelectorAll('td');
        cells.forEach((cell, index) => {
            csv += cell.textContent + (index < cells.length - 1 ? ',' : '\n');
        });
    });

    return csv;
}

function downloadFile(content, filename, type) {
    const blob = new Blob([content], { type });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = filename;
    document.body.appendChild(a);
    a.click();
    document.body.removeChild(a);
    URL.revokeObjectURL(url);
}

console.log('Student Tracking & Attendance System initialized');
