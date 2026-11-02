#include "LEDHttpHandler.h"

LEDHttpHandler::LEDHttpHandler(LEDService& ledService)
    : ledService_(ledService) {}

bool LEDHttpHandler::handle(const String& requestLine, HttpResponse& response) {
  if (requestLine.startsWith("GET /blink/fast ")) {
    ledService_.startBlinking(200);
  } else if (requestLine.startsWith("GET /blink/slow ")) {
    ledService_.startBlinking(1000);
  } else if (requestLine.startsWith("GET /blink/stop ")) {
    ledService_.stopBlinking();
  } else if (requestLine.startsWith("GET /status ")) {
    response = statusResponse();
    return true;
  } else {
    return false;
  }

  response = commandResponse();
  return true;
}

HttpResponse LEDHttpHandler::commandResponse() const {
  String body = "{\"success\":true,\"blinking\":";
  body += ledService_.isBlinking() ? "true" : "false";
  body += ",\"interval\":";
  body += ledService_.blinkInterval();
  body += "}";
  return {200, "OK", body};
}

HttpResponse LEDHttpHandler::statusResponse() const {
  String body = "{\"blinking\":";
  body += ledService_.isBlinking() ? "true" : "false";
  body += ",\"interval\":";
  body += ledService_.blinkInterval();
  body += ",\"led\":";
  body += ledService_.isOn() ? "\"ON\"" : "\"OFF\"";
  body += "}";
  return {200, "OK", body};
}
