
import { loadNavbar } from './common';

window.addEventListener('DOMContentLoaded', async () => {
    await loadNavbar();  // Shared function to load navbar and set active tab
});

function set_temperature(index: number, temperature: number)
{
    const htmlSpan = document.getElementById(`temperatureValue${index}`) as HTMLSpanElement;

    if (htmlSpan) {
        htmlSpan.textContent = `${temperature}`;
    }
}

function setFanSpeed(fan: number, speed: number, transmit: boolean, setSlider: boolean) {
    const htmlSpan = document.getElementById(`fanSpeedValue${fan}`) as HTMLSpanElement;

    if (htmlSpan) {
        htmlSpan.textContent = `${speed}`;
    }

    if (transmit) {
        fetch(`/set_fan_speed/${fan}?value=${speed}`, { method: 'GET' })
            .catch((error) => console.error('Error setting fan speed:', error));
    }

    if (setSlider)
    {
        const fanSpeedSlider = document.getElementById(`fanSpeedSlider${fan}`) as HTMLInputElement;
        if (fanSpeedSlider)
        {
            fanSpeedSlider.value = speed.toString();
        }
    }
}

// Listen to slider change and send value to ESP32
for (let i = 1; i <= 2; i++) {
    const fanSpeedSlider = document.getElementById(`fanSpeedSlider${i}`) as HTMLInputElement;
    if (fanSpeedSlider) {
        fanSpeedSlider.addEventListener('input', function () {
            const fanSpeed = Number(this.value);
            setFanSpeed(i, fanSpeed, true, false);
        });
    }
}

interface EventData {
    fan_speed1: number;
    fan_speed2: number;
    temperature1: number;
    temperature2: number;
    temperature3: number;
    temperature4: number;
}

// EventSource for receiving live updates
const evtSource = new EventSource('/events');

evtSource.onmessage = function (event: MessageEvent) {
    const data = JSON.parse(event.data) as EventData;

    setFanSpeed(1, data.fan_speed1, false, true);
    setFanSpeed(2, data.fan_speed2, false, true);
    set_temperature(1, data.temperature1);
    set_temperature(2, data.temperature2);
    set_temperature(3, data.temperature3);
    set_temperature(4, data.temperature4);
};

evtSource.onerror = function (err) {
    console.error('EventSource failed:', err);
};
