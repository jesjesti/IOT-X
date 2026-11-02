#include <Arduino.h>

#include "ApiRouter.h"
#include "HttpServer.h"
#include "LEDController.h"
#include "LEDService.h"
#include "WiFiManager.h"

namespace {

WiFiManager wifiManager;
LEDController ledController;
LEDService ledService(ledController);
ApiRouter apiRouter(ledService, wifiManager);
HttpServer httpServer(apiRouter);

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

  wifiManager.begin();
  httpServer.begin();
}

void loop() {
  wifiManager.update();

  httpServer.handleClient();
  ledService.update();
}
