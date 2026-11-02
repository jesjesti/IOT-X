This project contains experiments for the Arduino UNO R4 WiFi. Open the
`firmware` directory as a PlatformIO project in VS Code.

## Firmware structure

- `firmware/src/main.cpp` is the only Arduino entry point.
- `firmware/src/network/` contains Wi-Fi, HTTP transport, API routing, and
  feature-specific HTTP handlers.
- `firmware/src/controllers/` contains hardware-facing controllers only.
- `firmware/src/services/` contains application behavior and business logic.
- `firmware/include/` contains the module interfaces.

HTTP routes delegate to feature handlers, which call application services;
services use controllers to operate hardware. Add new functionality as its
own service and handler instead of adding business logic to the HTTP server.
Sensor, relay, MQTT, and OTA modules can follow the same boundaries when
implemented.

## Wi-Fi setup

Wi-Fi credentials are provisioned by the React app and saved in the board's
EEPROM; no credentials are compiled into firmware. On first boot (or after
resetting the saved credentials), the board starts an open setup network named
`IIOT-X-WiFi` at `192.168.4.1`.

1. Connect the computer to `IIOT-X-WiFi`.
2. Start the app from `arduino-UNO-R4-WiFi/client-app` with `npm install` and
   `npm run dev`, then open the Vite URL on that same computer.
3. Use **Scan networks**, choose the home Wi-Fi, enter its password, and save.
   The setup network is open, so credentials are sent over an unencrypted
   local connection; use it only in a trusted environment.
4. After the app confirms the save, connect the computer to the home Wi-Fi and
   update the Arduino address in the app to the board's home-network IP.

The app's **Reset Wi-Fi credentials** action erases the EEPROM record and
restarts setup mode. Connect the computer to `IIOT-X-WiFi` again to provision
the board. The status/scan/provision/reset API routes are documented in the
React app.

Open the PlatformIO Serial Monitor at 115200 baud and press RESET to see
startup messages. The firmware waits up to five seconds for the USB serial
connection before continuing. It prints the active mode and IP address, but
never logs the saved password.

## LED behavior

The LED is initially off. To start blinking at boot, uncomment
`ledService.startBlinking(200);` in `firmware/src/main.cpp`. The HTTP endpoints
also control it:

- `GET /blink/fast` — blink every 200 ms
- `GET /blink/slow` — blink every 1000 ms
- `GET /blink/stop` — stop blinking
- `GET /status` — return the current LED state
