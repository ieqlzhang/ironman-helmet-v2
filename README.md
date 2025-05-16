# Ironman Helmet Project

An ESP32-based Ironman helmet controller with LED animations and sensor integration.

## Hardware
- ESP32-C3 microcontroller
- MG90S servos (helmet movement)
- LED eyes (WS2812B)
- MPU6050 accelerometer/gyroscope
- Sound module (DFPlayer Mini)
- ASR PRO voice recognition module with speaker (Work-In-Progress)

## Installation
1. Install [PlatformIO](https://platformio.org/) extension in VSCode
2. Clone this repository
3. Connect hardware components as per wiring diagram

```bash
git clone https://github.com/ieqlzhang/ironman_helmet_v1.git
cd ironman_helmet_v1
pio run
```

## Usage
Upload the firmware to ESP32:
```bash
pio run -t upload
```

Control modes:
- Button press cycles through LED eye animation patterns
- MG90S servos control helmet faceplate movement
- Voice control system using ASR PRO module (Work-In-Progress)

## License
MIT License - See LICENSE for details
