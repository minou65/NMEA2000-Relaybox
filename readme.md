
# NMEA2000-Relaybox

<img src="img/garmin.jpeg" width="600" alt="Garmin plotter example">

## Overview

The NMEA2000-Relaybox is a flexible and robust device for controlling and monitoring relays over the NMEA2000 network. It is designed for seamless integration with marine electronics, enabling reliable management of electrical components on your vessel.
Relays can also be switched directly from the web interface by clicking the LED button. Additionally, an off-delay (drop-out time) can be configured for each relay, allowing it to automatically switch off after a specified period.

**Key Features:**
- Supports up to 8 relays per device (expandable across multiple devices)
- CAN bus communication via MCP2562
- Web-based configuration interface
- Compatible with plotters supporting PGN 127501 (Binary Switch Bank Status) and PGN 127502 (Switch Bank Control)
- Power supply via NMEA2000 bus

---

## Table of Contents

- [NMEA2000-Relaybox](#nmea2000-relaybox)
  - [Overview](#overview)
  - [Table of Contents](#table-of-contents)
  - [Other NMEA2000 Projects](#other-nmea2000-projects)
  - [Hardware](#hardware)
    - [ESP32\_Relay\_X2 Pinout](#esp32_relay_x2-pinout)
    - [ESP32\_Relay\_X8 Pinout](#esp32_relay_x8-pinout)
  - [Libraries Used](#libraries-used)
  - [Wiring Diagram](#wiring-diagram)
  - [Programming](#programming)
  - [Case and Enclosure](#case-and-enclosure)
  - [Configuration](#configuration)
    - [System Configuration](#system-configuration)
    - [NMEA Configuration](#nmea-configuration)
    - [Relay Configuration](#relay-configuration)
  - [Firmware Update](#firmware-update)
  - [Blink Codes](#blink-codes)
  - [Reset](#reset)
  - [Default Credentials](#default-credentials)
  - [Default IP Address](#default-ip-address)

---

## Other NMEA2000 Projects

In addition to the relay box, the following NMEA2000 projects are also available from the same author:
- [Fluid Level Sensor](https://github.com/minou65/NMEA2000-FluidLevel)
- [DS1820 Temperature Monitor](https://github.com/minou65/NMEA2000-DS1820)
- [BME280 Environmental Sensor](https://github.com/minou65/NMEA2000-BME280)
- [Battery Monitor](https://github.com/minou65/NMEA2000-BatteryMonitor)

---

## Hardware

This project uses either the ESP32_Relay_X2 or ESP32_Relay_X8 board as the base. The MCP2562 chip is connected to the expansion header for CAN bus communication. Power is supplied directly from the NMEA2000 bus. Each relay can switch up to 10A at 30V.

### ESP32_Relay_X2 Pinout
<img src="img/2xrelay.png" width="300" alt="ESP32_Relay_X2">

| GPIO | Output      |
|------|-------------|
| 16   | Relay 1     |
| 17   | Relay 2     |
| 23   | Internal LED|

### ESP32_Relay_X8 Pinout
<img src="img/16xrelay.png" width="300" alt="ESP32_Relay_X8">

| GPIO | Output      |
|------|-------------|
| 32   | Relay 1     |
| 33   | Relay 2     |
| 25   | Relay 3     |
| 26   | Relay 4     |
| 27   | Relay 5     |
| 14   | Relay 6     |
| 12   | Relay 7     |
| 13   | Relay 8     |
| 23   | Internal LED|

---

## Libraries Used

- [NMEA2000](https://github.com/ttlappalainen/NMEA2000)
- [NMEA2000_ESP32](https://github.com/ttlappalainen/NMEA2000_esp32)
- [AsyncTCP (3.2.6)](https://github.com/mathieucarbou/AsyncTCP)
- [ESPAsyncWebServer (3.3.12)](https://github.com/mathieucarbou/ESPAsyncWebServer)
- [WebSerial (2.0.7)](https://github.com/ayushsharma82/WebSerial)
- [IotWebConf](https://github.com/minou65/IotWebConf)
- [IotWebConfAsync (1.0.2)](https://github.com/minou65/IotWebConfAsync)
- [IotWebRoot](https://github.com/minou65/IotWebRoot)

---

## Wiring Diagram

<img src="sch/relaybox.png" width="600" alt="Wiring diagram">

---

## Programming

To program the ESP32 Relay Box for the first time, you will need a hardware programmer such as the ESP-Prog. The image below shows the pinout of the expansion header.

**Firmware Upload Procedure:**

1. Connect the ESP-Prog (or compatible programmer) to the expansion header as shown in the pinout diagram.
2. To enter programming mode:
  - Press and hold the I00 (GPIO0) button.
  - While holding I00, press the EN (Enable/Reset) button.
  - Release the EN button, then release the I00 button.
  - you will see the following in the Serial output
  ```cpp
  ets Jul 29 2019 12:21:46

rst:0x1 (POWERON_RESET),boot:0x3 (DOWNLOAD_BOOT(UART0/UART1/SDIO_REI_REO_V2))
waiting for download
  ```
3. Start the firmware upload from your development environment (e.g., Arduino IDE, PlatformIO).
4. Once the upload is complete, press the EN button again to restart the device.

<img src="img/esp32-pinout.jpg" width="300" alt="ESP32 expansion header pinout">

---

## Case and Enclosure

STL files for 3D printing the case and lid are available in the `stl` folder.

<img src="img/relaybox-case.jpg" width="300" alt="Case"> <img src="img/relaybox-lid.jpg" width="300" alt="Lid">
<img src="img/relaybox-closed.jpg" width="300" alt="Closed"> <img src="img/relaybox-open.jpg" width="300" alt="Open">

---

## Configuration

After the first boot, some values must be configured. Items marked with * are mandatory.

### System Configuration

- **Thing Name**: Assign a unique, descriptive name to the device (e.g., `relays_engine_room`). Use only English letters and underscores.
- **AP Password**: Set a secure password (8–32 characters, at least 12 recommended, with 3 character classes).*
- **WiFi SSID**: Name of the WiFi network to connect to.*
- **WiFi Password**: Password for the WiFi network above.*
- **AP Offline Mode (minutes)**: Set how long the AP should remain enabled after startup (0 = always enabled, up to 30 minutes).

### NMEA Configuration

- **Instance**: Should be unique for each device sending this PGN.*
- **SID**: Sequence identifier (usually 255).
- **Switch Bank Address**: Unique identifier for each device on the NMEA2000 network (up to 254 devices).

<img src="img/device-address.jpg" width="700" alt="Device address">

- **Relay Address**: Identifies which relay is controlled. Each switch bank can control up to 28 relays. Normally, the relay address is 1. If you use multiple relay boxes in one switch bank, set the correct relay address accordingly.

<img src="img/relay-address.jpg" width="700" alt="Relay address">

### Relay Configuration

- **GPIO**: The GPIO pin used for the relay.
- **Off Time (s)**: Time in seconds before the relay switches off automatically. If set to 0 (default), the relay remains on indefinitely.

---

## Firmware Update

To update the firmware, open the Configuration page in the web interface and click on the Firmware Update link. Follow the on-screen instructions.

---

## Blink Codes

| Blinking Pattern | Meaning |
|------------------|---------|
| Rapid blinking (mostly on, short off) | Access Point mode: device creates its own WiFi network. |
| Alternating on/off blinking | Connecting to configured WiFi network. |
| Mostly off, short flash | Device is online. |
| Mostly off, long flash | Device is in offline mode. |

---

## Reset

To reset the device, pull the CONFIG_PIN (GPIO 13) to ground during startup. The device will use the initial password to build an AP (useful if the password is lost).

---

## Default Credentials

- **Username:** admin
- **Default password (when not connected to an AP):** 123456789

## Default IP Address

When in AP mode, the default IP address is `192.168.4.1`.

---

