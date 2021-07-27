# Hypnic Power Manager
The Hypnic board is designed to provide just enough power for a SBC to power down gracefully when power is lost. This is not supposed to replace a UPS - the Hypnic will provide approximately 20 seconds of uptime after power is lost. It provides a few additional features, including power on/off (via a button), external LED with power status, supports multiple Single Board Computers, including the Raspberry Pi.

The Hypnic is intentionally not "HAT"-shaped, instead it can be wired directly to the GPIO of your SBC or, in the case of the Raspberry Pi, be paired with the Hypnic Companion HAT.

This repository is the AVR source code for the Hypnic Power Manager board that provides power control and emergency shutdown power for single board computers.

Read more at the device page: [Hypnic Power Manager](https://github.com/gilphilbert/falk/tree/main/hypnic)