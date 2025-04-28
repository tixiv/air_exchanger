Replacement firmwares for the board set from KD Elektroniksysteme which run the AERA-EQONIC.

The original firmware was very complicated, I gave up on reverse engineering it because I think reimplementing it is easier. We are missing the original control Panel for this thing, and without it it's just a brick.

This repository is work in progress, but there is some basic functionality already over a commandline interface. This needs to be replaced with an ESP32 for IoT support in the future.

There are 4 seperate firmwares in here, which will talk over the RS485 bus.
1. The fan controller
2. The mainboard (it has the humidity sensor, not implemented yet, and a 230V shutdown relay)
3. The controller for switching the PTC heater
4. A custom busmater using the ATMega1280 which has two UARTs. One connects to a terminal on the PC, the second one connects to the RS485 bus to control the other 3 boards.
