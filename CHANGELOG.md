# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-05-08

### Added
- Initial release
- ESP32-based CAN bus control for RB-A-420-3 6-axis robotic arm
- Web interface for remote control
- Teaching and playback functionality
- Flash storage for preset poses
- Support for 6 joints + 1 gripper

### Features
- CAN communication with joint motors (ID 1-7)
- Real-time joint angle monitoring
- Record and playback teaching sequences
- Store up to 10 poses per teaching group
- 4 preset action groups
- Emergency stop function
- Web-based control panel

### Hardware
- Controller: ESP32 DevKit
- CAN Bus: TX=GPIO26, RX=GPIO27
- Communication: WiFi AP mode

### Software
- PlatformIO development environment
- ESP Async WebServer
- Arduino framework