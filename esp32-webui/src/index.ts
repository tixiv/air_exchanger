
import './styles.css';

import { loadNavbar } from './common';

window.addEventListener('DOMContentLoaded', async () => {
  await loadNavbar();  // Shared function to load navbar and set active tab
});


// Define the structure of the data returned from the /status endpoint
interface StatusData {
    fan_speed: number;
    temperature: number;
    humidity: number;
}

// Function to get the current fan speed from the ESP32
function updateStatus(): void {
    fetch('/status')
        .then((response) => response.json())
        .then((data: StatusData) => {
            const fanSpeedSlider = document.getElementById('fanSpeedSlider') as HTMLInputElement;
            const fanSpeedValue = document.getElementById('fanSpeedValue') as HTMLSpanElement;
            const temperatureValue = document.getElementById('temperatureValue') as HTMLSpanElement;
            const humidityValue = document.getElementById('humidityValue') as HTMLSpanElement;

            if (fanSpeedSlider && fanSpeedValue && temperatureValue && humidityValue) {
                fanSpeedSlider.value = data.fan_speed.toString();
                fanSpeedValue.textContent = `Fan Speed: ${data.fan_speed}%`;
                temperatureValue.textContent = `${data.temperature}`;
                humidityValue.textContent = `${data.humidity} %`;
            }
        })
        .catch((err) => console.error('Failed to fetch status:', err));
}

// Listen to slider change and send value to ESP32
const fanSpeedSlider = document.getElementById('fanSpeedSlider') as HTMLInputElement;
if (fanSpeedSlider) {
    fanSpeedSlider.addEventListener('input', function () {
        const fanSpeed = this.value;
        const fanSpeedValue = document.getElementById('fanSpeedValue') as HTMLSpanElement;

        if (fanSpeedValue) {
            fanSpeedValue.textContent = `Fan Speed: ${fanSpeed}%`;
        }

        // Optionally, send the new fan speed to the backend
        fetch(`/set_fan_speed?value=${fanSpeed}`, { method: 'GET' })
            .catch((error) => console.error('Error setting fan speed:', error));
    });
}

// On page load, get the initial fan speed and temperature
window.onload = function () {
    updateStatus();
};

// Call every 5 seconds
setInterval(updateStatus, 5000);

// EventSource for receiving live updates
const evtSource = new EventSource('/events');

evtSource.onmessage = function (event: MessageEvent) {
    const data = JSON.parse(event.data);
    const temperatureValue = document.getElementById('temperatureValue') as HTMLSpanElement;

    if (temperatureValue) {
        temperatureValue.textContent = `${data.temperature}`;
    }
};

evtSource.onerror = function (err) {
    console.error('EventSource failed:', err);
};
