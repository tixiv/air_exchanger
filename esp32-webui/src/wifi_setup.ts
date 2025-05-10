
import { loadNavbar } from './common';

window.addEventListener('DOMContentLoaded', async () => {
    await loadNavbar();  // Shared function to load navbar and set active tab
});

interface AP {
    ssid: string;
    rssi: number;
}

// Fetch available networks
fetch('/scan')
    .then((response: Response) => response.json())
    .then((data: AP[]) => {
        const ssidSelect = document.getElementById('ssid') as HTMLSelectElement;
        data.forEach((ap: AP) => {
            const option = document.createElement('option');
            option.value = ap.ssid;
            option.text = `${ap.ssid} (${ap.rssi} dBm)`;
            ssidSelect.appendChild(option);
        });
    })
    .catch((err: Error) => {
        console.error("Failed to load Wi-Fi list:", err);
        const statusElement = document.getElementById('status');
        if (statusElement) {
            statusElement.textContent = "Error loading networks.";
        }
    });

// Handle form submit
document.getElementById('wifiForm')?.addEventListener('submit', function (e: Event) {
    e.preventDefault();

    const ssidElement = document.getElementById('ssid') as HTMLSelectElement;
    const passwordElement = document.getElementById('password') as HTMLInputElement;

    const ssid = ssidElement.value;
    const password = passwordElement.value;

    fetch('/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid, password })
    })
        .then((res: Response) => res.text())
        .then((msg: string) => {
            const statusElement = document.getElementById('status');
            if (statusElement) {
                statusElement.textContent = msg;
            }
        })
        .catch((err: Error) => {
            const statusElement = document.getElementById('status');
            if (statusElement) {
                statusElement.textContent = "Failed to send credentials.";
            }
        });
});