This project contains experiments for the Arduino UNO R4 WiFi. Open the
`board` directory as a PlatformIO project in VS Code.

## Firmware structure

- `board/src/main.cpp` is the only Arduino entry point.
- `board/src/network/` contains Wi-Fi, HTTP transport, API routing, and
  feature-specific HTTP handlers.
- `board/src/controllers/` contains hardware-facing controllers only.
- `board/src/services/` contains application behavior and business logic.
- `board/include/` contains the module interfaces.

HTTP routes delegate to feature handlers, which call application services;
services use controllers to operate hardware. Add new functionality as its
own service and handler instead of adding business logic to the HTTP server.
Sensor, relay, MQTT, and OTA modules can follow the same boundaries when
implemented.

## Wi-Fi setup

Copy `board/include/Secrets.h.example` to `board/include/Secrets.h` and set
your network name and password there. `Secrets.h` is ignored by Git.

Open the PlatformIO Serial Monitor at 115200 baud and press the board's RESET
button to see startup messages. The firmware waits up to five seconds for the
USB serial connection before continuing. It prints whether credentials are
missing, when it starts connecting, and the board's IP after a successful
connection.

## LED behavior

The LED is initially off. To start blinking at boot, uncomment
`ledService.startBlinking(200);` in `board/src/main.cpp`. The HTTP endpoints
also control it:

- `GET /blink/fast` — blink every 200 ms
- `GET /blink/slow` — blink every 1000 ms
- `GET /blink/stop` — stop blinking
- `GET /status` — return the current LED state
