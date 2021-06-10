<a href="https://github.com/ezralazuardy/cats/actions/workflows/platformio-ci.yml">
  <img src="https://img.shields.io/github/workflow/status/ezralazuardy/cats/PlatformIO%20CI?label=build" alt="Build" target="_blank" rel="noopener noreferrer">
</a>

  <a href="https://github.com/ezralazuardy/cats/actions/workflows/codacy-analysis.yml">
    <img src="https://img.shields.io/github/workflow/status/ezralazuardy/cats/Codacy%20Security%20Scan?label=security" alt="Security" target="_blank" rel="noopener noreferrer">
  </a>

<a href="https://github.com/ezralazuardy/cats/releases">
  <img src="https://img.shields.io/github/v/release/ezralazuardy/cats" alt="Releases" target="_blank" rel="noopener noreferrer">
</a>

<a href="https://github.com/ezralazuardy/cats/blob/master/LICENSE">
  <img src="https://img.shields.io/github/license/ezralazuardy/cats" alt="Releases" target="_blank" rel="noopener noreferrer">
</a>

# 😷 CATS

CATS is a Contactless Automatic Thermal Scanner that uses Arduino UNO R3 and HTTP Request to automate thermal scanning purposes.

This project use [PlatformIO](https://platformio.org) for compiler and package manager.

<br/>

## 🔨 Build

Open PlatformIO Core CLI and run:

```
pio lib install
pio run
````

<br/>

## 📤 Upload to Arduino Board

Open PlatformIO CLI and connect your Arduino board to PC, then run:

```
pio run --target upload
````

<br/>

## 🗺️ Topology

<p align="center"><img src="https://i.ibb.co/SrF1yCj/topology.png" alt="topology" height="200"/></p>

## 📦 Dependencies

- [paulstoffregen/Ethernet](https://platformio.org/lib/show/134/Ethernet/examples?file=LinkStatus.ino)
- [marcoschwartz/LiquidCrystal_I2C](https://platformio.org/lib/show/576/LiquidCrystal_I2C)
- [adafruit/Adafruit MLX90614 Library](https://platformio.org/lib/show/782/Adafruit%20MLX90614%20Library)
- [arduino-libraries/ArduinoHttpClient](https://platformio.org/lib/show/798/ArduinoHttpClient)

<br/>

## ⚙️ Hardwares

- Arduino UNO R3 Atmega 328P 16U2
- Arduino Ethernet Shield W5100
- IR Obstacle Avoidance Sensor Module
- MLX90614 GY-906-BAA Contactless Temperature Sensor Module
- Active Buzzer 5v
- Passive Buzzer 5v
- LCD 16x2 (with I2C Serial Interface Module)
- LED Red
- LED Green
- Metal Film Resistor 0,25W 1% 220 ohm (2 pieces)
- LAN Cable (RJ45)
- USB 2.0 Data Cable Type A to B Male to Male
- Jumper Cable Male to Male
- Jumper Cable Male to Female
