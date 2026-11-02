# IOT-X

## Overview

This repository contains R&D code for Internet of Things (IoT), robotics, and related embedded systems projects. It provides a collection of client applications, device firmware, reference implementations, and practical examples to support hardware prototyping and user interface integrations across various platforms and use cases.

## Directory map

```text
IOT-X/
├── .github/
│   └── agents/                         # Repository-specific Copilot agents and references
├── .vscode/                            # VS Code workspace settings
├── arduino-UNO-R4-WiFi/
│   ├── client-app/                     # React/Vite control app for the Arduino
│   │   ├── public/                     # Static assets
│   │   └── src/                        # App source and assets
│   ├── firmware/                       # PlatformIO firmware for Arduino UNO R4 WiFi
│   │   ├── include/                    # Firmware interfaces and headers
│   │   ├── lib/                        # Project-local PlatformIO libraries
│   │   ├── src/
│   │   │   ├── controllers/            # Hardware-facing controllers
│   │   │   ├── network/                # Wi-Fi, HTTP server, routes, and handlers
│   │   │   └── services/               # Application and device behavior
│   │   └── test/                       # Firmware tests
│   └── readme.md                       # Arduino project setup and API notes
├── robo-hand/
│   ├── client-app/                     # React control app for the robot
│   │   ├── public/                     # Static assets
│   │   └── src/                        # App source
│   ├── device/
│   │   ├── ESP32CONTROL/               # ESP32 robot control firmware
│   │   └── SERVOTEST/                  # Servo sweep test sketch
│   └── README.md                       # Robot project setup and hardware notes
├── LICENSE
└── README.md
```

Generated build output and local dependency folders (such as `dist/`, `.pio/`,
`node_modules/`, and `.vite/`) are not part of the source layout.

## Contributing

To contribute to this project:

- Create a feature branch and open a pull request with detailed information
- Document hardware configurations, board versions, SDK versions, and hardware schematics
- Write clean, minimal, and thoroughly commented code to ensure reproducibility and maintainability

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
