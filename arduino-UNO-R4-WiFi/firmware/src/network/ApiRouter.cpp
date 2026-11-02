#include "ApiRouter.h"

ApiRouter::ApiRouter(LEDService& ledService, WiFiManager& wifiManager)
    : ledHandler_(ledService), wifiHandler_(wifiManager) {}

HttpResponse ApiRouter::dispatch(const String& requestLine,
                                 const String& body) {
  HttpResponse response{};
  if (wifiHandler_.handle(requestLine, body, response)) {
    return response;
  }
  if (ledHandler_.handle(requestLine, response)) {
    return response;
  }

  return {404, "Not Found", "{\"error\":\"Unknown endpoint\"}"};
}
