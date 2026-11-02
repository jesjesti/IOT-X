# IOT-X device app

## Run the app

Start the app on the same computer that will connect to the Arduino:

```sh
npm install
npm run dev
```

On first boot, connect the computer to the board's open `IIOT-X-WiFi` access
point. The app defaults to the setup IP `192.168.4.1`. Select **Scan networks**,
choose the home Wi-Fi, enter its password, and select **Save and connect**.
Once the board switches to station mode, connect the computer to the home
network and enter the Arduino's new IP address. **Reset Wi-Fi credentials**
erases the saved network and starts setup mode again.

The setup access point is open, and provisioning credentials are sent over
unencrypted HTTP. Provision only in a trusted environment.

The Vite development proxy is restricted to requests from localhost and private
IPv4 addresses. The UI sends requests to that proxy to avoid browser CORS
restrictions; the proxy forwards them to the Arduino.

## Device API

- `GET /wifi/status` — current setup/station mode, SSID, connection state, and IP
- `GET /wifi/networks` — scan results while in setup mode
- `POST /wifi/credentials` — JSON body: `{"ssid":"...","password":"..."}`
- `DELETE /wifi/credentials` — erase saved Wi-Fi details and restart setup
- `GET /blink/fast`, `GET /blink/slow`, `GET /blink/stop`, `GET /status` — LED
  controls and status
