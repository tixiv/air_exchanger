const express = require('express');
const path = require('path');
const app = express();
const port = 3000;


// Set Cache-Control headers for JavaScript and CSS files to prevent caching
app.use(express.static(path.join(__dirname, 'dist'), {
    maxAge: '0', // Set cache duration to 0 for development (no caching)
    setHeaders: function (res, path) {
        res.setHeader('Cache-Control', 'no-store'); // Don't cache JS/CSS
    }
}));


// Mock /status endpoint
app.get('/status', (req, res) => {
    res.json({
        fan_speed: 50,
        temperature: 23.5,
        humidity: 60
    });
});

// Mock /set_fan_speed endpoint
app.get('/set_fan_speed', (req, res) => {
    const fanSpeed = req.query.value;
    console.log(`Fan speed set to: ${fanSpeed}`);
    res.sendStatus(200);
});

// Mock EventSource (just sends temperature updates every 5 seconds)
app.get('/events', (req, res) => {
    res.setHeader('Content-Type', 'text/event-stream');
    res.setHeader('Cache-Control', 'no-cache');
    res.setHeader('Connection', 'keep-alive');
    res.flushHeaders();

    setInterval(() => {
        res.write(`data: {"temperature": ${Math.random() * 30}}\n\n`);
    }, 5000);
});

// Redirect from '/' to '/index.html'
app.get('/', (req, res) => {
    res.redirect('/index.html');
});

// Start the server
app.listen(port, () => {
    console.log(`Mock server listening at http://localhost:${port}`);
});
