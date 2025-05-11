
import { loadNavbar } from './common';

let evtSource: EventSource;

window.addEventListener('DOMContentLoaded', async () => {
    await loadNavbar();

    const powerSwitch = document.getElementById('systemPowerSwitch') as HTMLInputElement;
    if (powerSwitch) {
        powerSwitch.addEventListener('change', () => {
            setPower(powerSwitch.checked, true, false);
        });
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

    const heaterSlider = document.getElementById('heaterSlider') as HTMLInputElement;
    if (heaterSlider) {
        heaterSlider.addEventListener('input', function () {
            const value = Number(this.value);
            setHeater(value, true, false);
        });
    }


    // EventSource for receiving live updates
    evtSource = new EventSource('/events');

    evtSource.onmessage = function (event: MessageEvent) {
        const data = JSON.parse(event.data) as EventData;

        setPower(data.power != 0, false, true);
        setHeater(data.heater, false, true);
        setFanSpeed(1, data.fan_speed1, false, true);
        setFanSpeed(2, data.fan_speed2, false, true);
        set_fan_rpm(1, data.fan_rpm1);
        set_fan_rpm(2, data.fan_rpm2);
        set_temperature(1, data.temperature1);
        set_temperature(2, data.temperature2);
        set_temperature(3, data.temperature3);
        set_temperature(4, data.temperature4);
        set_temperature(5, data.temperature5);
        evtSource.onerror = function (err) {
            console.error('EventSource failed:', err);
        };
    };
});

function setPower(power: boolean, transmit: boolean, setSwitch: boolean) {

    if (transmit) {
        fetch(`/set_power?value=${power ? 1:0}`, { method: 'GET' })
            .catch((error) => console.error('Error setting power:', error));
    }

    if (setSwitch) {
        const powerSwitch = document.getElementById('systemPowerSwitch') as HTMLInputElement;

        if (powerSwitch) {
            powerSwitch.checked = power;
        }
    }
}

function setFanSpeed(fan: number, speed: number, transmit: boolean, setSlider: boolean) {
    const htmlSpan = document.getElementById(`fanSpeedValue${fan}`) as HTMLSpanElement;

    if (htmlSpan) {
        htmlSpan.textContent = `${speed}`;
    }

    if (transmit) {
        fetch(`/set_fan_speed?fan=${fan}&value=${speed}`, { method: 'GET' })
            .catch((error) => console.error('Error setting fan speed:', error));
    }

    if (setSlider) {
        const fanSpeedSlider = document.getElementById(`fanSpeedSlider${fan}`) as HTMLInputElement;
        if (fanSpeedSlider) {
            fanSpeedSlider.value = speed.toString();
        }
    }
}

function setHeater(value: number, transmit: boolean, setSlider: boolean) {
    if (transmit) {
        fetch(`/set_heater?value=${value}`, { method: 'GET' })
            .catch((error) => console.error('Error setting fan speed:', error));
    }

    if (setSlider) {
        const slider = document.getElementById('heaterSlider') as HTMLInputElement;
        if (slider) {
            slider.value = value.toString();
        }
    }
}

function set_temperature(index: number, temperature: number) {
    const htmlSpan = document.getElementById(`temperatureValue${index}`) as HTMLSpanElement;

    if (htmlSpan) {
        htmlSpan.textContent = `${temperature}`;
    }
}

function set_fan_rpm(index: number, rpm: number) {
    const htmlSpan = document.getElementById(`fanRPM${index}`) as HTMLSpanElement;

    if (htmlSpan) {
        htmlSpan.textContent = `${rpm}`;
    }
}

interface EventData {
    power: number;
    heater: number;
    fan_speed1: number;
    fan_speed2: number;
    fan_rpm1: number;
    fan_rpm2: number;
    temperature1: number;
    temperature2: number;
    temperature3: number;
    temperature4: number;
    temperature5: number;
}
