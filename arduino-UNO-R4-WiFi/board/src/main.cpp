#include <Arduino.h>

#include "ApiRouter.h"
#include "HttpServer.h"
#include "LEDController.h"
#include "LEDService.h"
#include "WiFiManager.h"

#if __has_include("Secrets.h")
#include "Secrets.h"
#else
constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";
#endif

namespace {

WiFiManager wifiManager;
LEDController ledController;
LEDService ledService(ledController);
ApiRouter apiRouter(ledService);
HttpServer httpServer(apiRouter);
bool httpServerStarted = false;

}  // namespace

void setup() {
  Serial.begin(115200);

  const unsigned long serialWaitStartedAt = millis();
  while (!Serial && millis() - serialWaitStartedAt < 5000) {
    delay(10);
  }
  Serial.println("UNO R4 WiFi firmware starting.");

  ledService.begin();
  // Uncomment to start the standalone LED blink when the board boots.
  // ledService.startBlinking(200);

  wifiManager.begin(WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  wifiManager.update();

  if (wifiManager.isConnected()) {
    if (!httpServerStarted) {
      httpServer.begin();
      httpServerStarted = true;
    }
    httpServer.handleClient();
  }

  ledService.update();
}
