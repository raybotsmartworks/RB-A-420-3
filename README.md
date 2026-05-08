# RB-A-420-3 Robotic Arm Control

## Introduction

ESP32-based CAN bus control system for RB-A-420-3 6-axis robotic arm with web interface.

![Platform](https://img.shields.io/badge/Platform-ESP32-blue)
![Framework](https://img.shields.io/badge/Framework-PlatformIO-green)
![License](https://img.shields.io/badge/License-MIT-yellow)

## Hardware

- ESP32 DevKit
- RB-A-420-3 Robotic Arm (6-axis + gripper)
- CAN Bus Interface

## Software

- PlatformIO
- ESP Async WebServer
- Arduino Framework

## Features

- CAN bus communication with joint motors
- Web-based remote control
- Real-time status monitoring
- Teaching & playback
- Flash storage for presets
- Emergency stop

## Getting Started

### 1. Build & Upload

```bash
# Build
pio run

# Upload via USB
pio run --target upload

# Upload over network
pio run --target upload --upload-port 192.168.1.100
```

### 2. WiFi Configuration

Edit `include/config.h`:

```c
#define WIFI_SSID "YourWiFiName"
#define WIFI_PASSWORD "YourPassword"
```

### 3. Access Web Interface

Open browser and visit ESP32 IP address (default: `http://192.168.4.1` when in AP mode)

## Pin Configuration

| Function | GPIO |
|----------|------|
| CAN_TX | GPIO26 |
| CAN_RX | GPIO27 |
| CAN_EN | GPIO23 |

## File Structure

```
RB-A-420-3/
├── src/
│   ├── main.cpp       # Main program
│   ├── rclib.cpp     # Arm control library
│   ├── rclib.h       # Library header
│   └── web_pages.h   # Web interface
├── include/
│   └── config.h      # Configuration
├── platformio.ini    # Project config
├── README.md         # English docs
├── README_zh.md     # Chinese docs
└── LICENSE          # MIT License
```

## API Endpoints

| Endpoint | Parameter | Description |
|----------|-----------|-------------|
| `/` | GET | Web control page |
| `/cmd` | `action=xxx` | Execute command |
| `/status` | GET | JSON status |

**Commands:**
- `readJoints` - Read joint angles
- `record` - Record current pose
- `runTeach` - Run teaching sequence
- `clawOpen` / `clawClose` - Gripper control
- `preset1-4` - Run preset actions
- `emergency` - Emergency stop

## Documentation

- [English README](README.md)
- [中文说明](README_zh.md)
- [详细使用说明](RB-A-420-3机械臂使用及代码说明.md)

## License

MIT License - see [LICENSE](LICENSE) file.

## Support

For issues and questions, please open an Issue on GitHub.