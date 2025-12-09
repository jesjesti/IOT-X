## Repository layout

- - client-app/ — React control UI and demo web app
    - [package.json](client-app/package.json) — npm scripts and dependencies
    - [src/App.js](client-app/src/App.js) — main React UI and network calls (see [`sendCommand`](client-app/src/App.js) and [`sendServoAngle`](client-app/src/App.js))
    - [src/reportWebVitals.js](client-app/src/reportWebVitals.js)
    - [public/index.html](client-app/public/index.html)
    - [README](client-app/README.md)
  - device/
    - ESP32CONTROL/ — ESP32 HTTP control firmware
      - [ESP32CONTROL.ino](device/ESP32CONTROL/ESP32CONTROL.ino) — HTTP endpoints and motor/servo handlers (see [`handleRotorServo`](device/ESP32CONTROL/ESP32CONTROL.ino) and other handlers)
    - SERVOTEST/ — simple servo sweep example
      - [SERVOTEST.ino](device/SERVOTEST/SERVOTEST.ino)

## Quick start

1. Client app (development)

   - Open a terminal in [client-app](client-app).
   - Install deps and start:
     ```sh
     npm install
     npm start
     ```
   - The React app runs on http://localhost:3000 by default. It calls the robot AP at 192.168.4.1 (see [`sendCommand`](client-app/src/App.js)).

2. ESP32 firmware

   - Open [device/ESP32CONTROL/ESP32CONTROL.ino](device/ESP32CONTROL/ESP32CONTROL.ino) in the Arduino IDE or PlatformIO.
   - Select ESP32 board, port, compile and upload.
   - The firmware creates a Wi‑Fi access point `ESP32_Control_AP` and serves HTTP endpoints:
     - `/forward`, `/backward`, `/rotateLeft`, `/rotateRight` — movement press/hold handlers (see file)
     - `/rotor/servo?angle=<0-180>` — set rotor servo angle (handled by [`handleRotorServo`](device/ESP32CONTROL/ESP32CONTROL.ino))

3. Servo test
   - [device/SERVOTEST/SERVOTEST.ino](device/SERVOTEST/SERVOTEST.ino) demonstrates a continuous 0–180° sweep for testing hardware and wiring.

## How the pieces connect

- The React UI in [client-app/src/App.js](client-app/src/App.js) sends HTTP GET requests to the ESP32 AP IP (192.168.4.1). Movement is implemented as press-and-hold using [`sendCommand`](client-app/src/App.js), and servo control uses [`sendServoAngle`](client-app/src/App.js).
- The ESP32 firmware maps HTTP routes to motor/servo actions in [device/ESP32CONTROL/ESP32CONTROL.ino](device/ESP32CONTROL/ESP32CONTROL.ino).

## Development notes & conventions

- IP: client assumes the ESP32 is in AP mode with IP `192.168.4.1` (see [src/App.js](client-app/src/App.js)).
- Servo angles are clamped to 0–180° on both client and firmware; use integer degrees.
- Movement endpoints accept `?state=start|stop` for press-and-hold behavior.
- Use the example in [SERVOTEST.ino](device/SERVOTEST/SERVOTEST.ino) to validate servo wiring and power.

## Testing

- Run React unit tests in [client-app](client-app):
  ```sh
  npm test
  ```
