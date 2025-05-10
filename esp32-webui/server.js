
const express = require('express');
const path = require('path');
const cors = require('cors'); // Optional: If you are dealing with CORS issues
const bodyParser = require('body-parser');

const app = express();
const port = 3000;

// Middlewares
app.use(cors());
app.use(bodyParser.json()); // To parse JSON bodies in POST requests

// Set Cache-Control headers for JavaScript and CSS files to prevent caching
app.use(express.static(path.join(__dirname, 'dist'), {
    maxAge: '0', // Set cache duration to 0 for development (no caching)
    setHeaders: function (res, path) {
        res.setHeader('Cache-Control', 'no-store'); // Don't cache JS/CSS
    }
}));

let fanSpeed1 = 66;
let fanSpeed2 = 55;

// Mock /set_fan_speed endpoints
app.get('/set_fan_speed/1', (req, res) => {
    fanSpeed1 = req.query.value;
    console.log(`Fan speed 1 set to: ${fanSpeed1}`);
    res.sendStatus(200);
});

app.get('/set_fan_speed/2', (req, res) => {
    fanSpeed2 = req.query.value;
    console.log(`Fan speed 2 set to: ${fanSpeed2}`);
    res.sendStatus(200);
});

app.get('/events', (req, res) => {
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');
    res.flushHeaders(); // flush the headers to establish SSE

    const interval = setInterval(() => {
        const data = {
            fan_speed1: fanSpeed1,
            fan_speed2: fanSpeed2,
            temperature1: Math.random() * 60,
            temperature2: Math.random() * 30,
            temperature3: Math.random() * 30,
            temperature4: Math.random() * 30,
        };

        res.write(`data: ${JSON.stringify(data)}\n\n`);
    }, 2000);

    // Handle client disconnect
    req.on('close', () => {
        clearInterval(interval);
        res.end();
    });
});

// Mock data for available networks
const mockNetworks = [
    { ssid: 'Network1', rssi: -30 },
    { ssid: 'Network2', rssi: -50 },
    { ssid: 'Network3', rssi: -80 }
];

// Endpoint to get available networks
app.get('/scan', (req, res) => {
    // Simulate fetching available networks (you could replace this with real Wi-Fi scanning logic)
    res.json(mockNetworks);
});

// Endpoint to handle Wi-Fi connection
app.post('/connect', (req, res) => {
    const { ssid, password } = req.body;

    // Simulate connecting to Wi-Fi (replace with actual logic to connect to Wi-Fi)
    if (ssid && password) {
        res.send(`Successfully connected to ${ssid}.`);
    } else {
        res.status(400).send('Failed to connect: Missing SSID or password.');
    }
});

// Redirect from '/' to '/index.html'
app.get('/', (req, res) => {
    res.redirect('/index.html');
});

// Start the server
app.listen(port, () => {
    console.log(`Mock server listening at http://localhost:${port}`);
});
