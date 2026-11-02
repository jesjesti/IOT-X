#include "WiFiHttpHandler.h"

#include <WiFiS3.h>

namespace {

bool isWhitespace(char character) {
  return character == ' ' || character == '\t' || character == '\r' ||
         character == '\n';
}

int hexDigit(char character) {
  if (character >= '0' && character <= '9') {
    return character - '0';
  }
  if (character >= 'a' && character <= 'f') {
    return character - 'a' + 10;
  }
  if (character >= 'A' && character <= 'F') {
    return character - 'A' + 10;
  }
  return -1;
}

bool readCodeUnit(const String& json, int& position, uint16_t& codeUnit) {
  if (position + 4 > json.length()) {
    return false;
  }

  codeUnit = 0;
  for (int index = 0; index < 4; ++index) {
    const int digit = hexDigit(json[position++]);
    if (digit < 0) {
      return false;
    }
    codeUnit = static_cast<uint16_t>((codeUnit << 4) | digit);
  }
  return true;
}

void appendUtf8(String& value, uint32_t codePoint) {
  if (codePoint <= 0x7F) {
    value += static_cast<char>(codePoint);
  } else if (codePoint <= 0x7FF) {
    value += static_cast<char>(0xC0 | (codePoint >> 6));
    value += static_cast<char>(0x80 | (codePoint & 0x3F));
  } else if (codePoint <= 0xFFFF) {
    value += static_cast<char>(0xE0 | (codePoint >> 12));
    value += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
    value += static_cast<char>(0x80 | (codePoint & 0x3F));
  } else {
    value += static_cast<char>(0xF0 | (codePoint >> 18));
    value += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
    value += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
    value += static_cast<char>(0x80 | (codePoint & 0x3F));
  }
}

bool readJsonStringField(const String& json, const char* key, String& value) {
  String quotedKey = "\"";
  quotedKey += key;
  quotedKey += "\"";
  int position = json.indexOf(quotedKey);
  if (position < 0) {
    return false;
  }

  position += quotedKey.length();
  while (position < json.length() && isWhitespace(json[position])) {
    ++position;
  }
  if (position >= json.length() || json[position++] != ':') {
    return false;
  }
  while (position < json.length() && isWhitespace(json[position])) {
    ++position;
  }
  if (position >= json.length() || json[position++] != '\"') {
    return false;
  }

  value = "";
  while (position < json.length()) {
    const char character = json[position++];
    if (character == '\"') {
      return true;
    }
    if (static_cast<uint8_t>(character) < 0x20) {
      return false;
    }
    if (character != '\\') {
      value += character;
      continue;
    }
    if (position >= json.length()) {
      return false;
    }

    switch (json[position++]) {
      case '\"':
        value += '\"';
        break;
      case '\\':
        value += '\\';
        break;
      case '/':
        value += '/';
        break;
      case 'b':
        value += '\b';
        break;
      case 'f':
        value += '\f';
        break;
      case 'n':
        value += '\n';
        break;
      case 'r':
        value += '\r';
        break;
      case 't':
        value += '\t';
        break;
      case 'u': {
        uint16_t firstCodeUnit = 0;
        if (!readCodeUnit(json, position, firstCodeUnit)) {
          return false;
        }
        uint32_t codePoint = firstCodeUnit;
        if (firstCodeUnit >= 0xD800 && firstCodeUnit <= 0xDBFF) {
          if (position + 2 > json.length() || json[position++] != '\\' ||
              json[position++] != 'u') {
            return false;
          }
          uint16_t secondCodeUnit = 0;
          if (!readCodeUnit(json, position, secondCodeUnit) ||
              secondCodeUnit < 0xDC00 || secondCodeUnit > 0xDFFF) {
            return false;
          }
          codePoint = 0x10000 +
                      ((static_cast<uint32_t>(firstCodeUnit) - 0xD800) << 10) +
                      (secondCodeUnit - 0xDC00);
        } else if (firstCodeUnit >= 0xDC00 && firstCodeUnit <= 0xDFFF) {
          return false;
        }
        appendUtf8(value, codePoint);
        break;
      }
      default:
        return false;
    }
  }
  return false;
}

String jsonQuoted(const String& value) {
  String result = "\"";
  for (unsigned int index = 0; index < value.length(); ++index) {
    const char character = value[index];
    if (character == '\"' || character == '\\') {
      result += '\\';
    }
    if (static_cast<uint8_t>(character) < 0x20) {
      result += ' ';
    } else {
      result += character;
    }
  }
  result += "\"";
  return result;
}

}  // namespace

WiFiHttpHandler::WiFiHttpHandler(WiFiManager& wifiManager)
    : wifiManager_(wifiManager) {}

bool WiFiHttpHandler::handle(const String& requestLine, const String& body,
                             HttpResponse& response) {
  if (requestLine.startsWith("GET /wifi/status ")) {
    response = statusResponse();
    return true;
  }
  if (requestLine.startsWith("GET /wifi/networks ")) {
    if (!wifiManager_.isProvisioning()) {
      response = {409, "Conflict",
                  "{\"error\":\"Network scanning is only available in provisioning mode\"}"};
    } else {
      String networks;
      if (wifiManager_.scanNetworks(networks)) {
        response = {200, "OK", networks};
      } else {
        response = {503, "Service Unavailable",
                    "{\"error\":\"Could not scan for Wi-Fi networks\"}"};
      }
    }
    return true;
  }
  if (requestLine.startsWith("POST /wifi/credentials ")) {
    response = credentialsResponse(body);
    return true;
  }
  if (requestLine.startsWith("DELETE /wifi/credentials ")) {
    wifiManager_.resetCredentials();
    response = {200, "OK",
                "{\"success\":true,\"mode\":\"provisioning\"}"};
    return true;
  }
  return false;
}

HttpResponse WiFiHttpHandler::statusResponse() const {
  String body = "{\"mode\":\"";
  body += wifiManager_.isProvisioning() ? "provisioning" : "station";
  body += "\",\"connected\":";
  body += wifiManager_.isConnected() ? "true" : "false";
  body += ",\"ssid\":";
  body += jsonQuoted(wifiManager_.currentSsid());
  body += ",\"ip\":";
  body += jsonQuoted(wifiManager_.isProvisioning() ? WiFi.softAPIP().toString()
                                                  : WiFi.localIP().toString());
  body += "}";
  return {200, "OK", body};
}

HttpResponse WiFiHttpHandler::credentialsResponse(const String& body) {
  String ssid;
  String password;
  if (!readJsonStringField(body, "ssid", ssid) ||
      !readJsonStringField(body, "password", password)) {
    return {400, "Bad Request",
            "{\"error\":\"Request must include string ssid and password fields\"}"};
  }

  if (!wifiManager_.saveCredentials(ssid, password)) {
    return {400, "Bad Request",
            "{\"error\":\"SSID must be 1-32 bytes; password must be empty or 8-63 bytes\"}"};
  }

  return {200, "OK",
          "{\"success\":true,\"mode\":\"station\",\"message\":\"Credentials saved; switching to station mode\"}"};
}
