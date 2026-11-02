#include "WiFiManager.h"

#include <Arduino.h>
#include <EEPROM.h>
#include <WiFiS3.h>

namespace {

constexpr char kProvisioningSsid[] = "IIOT-X-WiFi";
constexpr uint32_t kCredentialsMagic = 0x49584F54;
constexpr int kMagicAddress = 0;
constexpr int kSsidLengthAddress = sizeof(uint32_t);
constexpr int kPasswordLengthAddress = kSsidLengthAddress + 1;
constexpr int kCredentialsAddress = kPasswordLengthAddress + 1;
constexpr int kCredentialsStorageSize = kCredentialsAddress + 32 + 63;
constexpr unsigned long kReconnectIntervalMs = 10000;
constexpr unsigned long kStatusCheckIntervalMs = 1000;

bool hasUsableIp(const IPAddress& address) {
  return address[0] != 0 || address[1] != 0 || address[2] != 0 ||
         address[3] != 0;
}

String jsonQuoted(const String& value) {
  String result = "\"";
  for (unsigned int index = 0; index < value.length(); ++index) {
    const char character = value[index];
    switch (character) {
      case '\"':
        result += "\\\"";
        break;
      case '\\':
        result += "\\\\";
        break;
      case '\b':
        result += "\\b";
        break;
      case '\f':
        result += "\\f";
        break;
      case '\n':
        result += "\\n";
        break;
      case '\r':
        result += "\\r";
        break;
      case '\t':
        result += "\\t";
        break;
      default:
        if (static_cast<uint8_t>(character) < 0x20) {
          result += ' ';
        } else {
          result += character;
        }
    }
  }
  result += "\"";
  return result;
}

}  // namespace

void WiFiManager::begin() {
  if (loadCredentials()) {
    mode_ = Mode::kStation;
    connect();
  } else {
    startProvisioning();
  }
}

void WiFiManager::update() {
  if (pendingAction_ != PendingAction::kNone) {
    const PendingAction action = pendingAction_;
    pendingAction_ = PendingAction::kNone;
    if (action == PendingAction::kProvisioning) {
      startProvisioning();
    } else {
      mode_ = Mode::kStation;
      connectionReady_ = false;
      connectionLogged_ = false;
      connect();
    }
    return;
  }

  if (mode_ == Mode::kProvisioning) {
    if (WiFi.status() != WL_AP_LISTENING &&
        millis() - lastAttemptMs_ >= kReconnectIntervalMs) {
      startProvisioning();
    }
    return;
  }

  const unsigned long now = millis();
  if (now - lastStatusCheckMs_ < kStatusCheckIntervalMs) {
    return;
  }
  lastStatusCheckMs_ = now;

  if (WiFi.status() == WL_CONNECTED) {
    const IPAddress address = WiFi.localIP();
    if (hasUsableIp(address)) {
      connectionReady_ = true;
      ipWarningLogged_ = false;
      if (!connectionLogged_) {
        Serial.println("Wi-Fi connected.");
        Serial.print("Arduino IP Address: ");
        Serial.println(address);
        connectionLogged_ = true;
      }
      return;
    }

    connectionReady_ = false;
    if (connectionLogged_) {
      Serial.println("Wi-Fi connection lost its IP address.");
      connectionLogged_ = false;
    }
    if (!ipWarningLogged_) {
      Serial.println(
          "Wi-Fi connected, but DHCP has not assigned an IP address yet.");
      ipWarningLogged_ = true;
    }
  } else {
    connectionReady_ = false;
    ipWarningLogged_ = false;
    if (connectionLogged_) {
      Serial.println("Wi-Fi connection lost.");
      connectionLogged_ = false;
    }
  }

  if (now - lastAttemptMs_ >= kReconnectIntervalMs) {
    connect();
  }
}

bool WiFiManager::isConnected() const {
  return connectionReady_;
}

bool WiFiManager::isProvisioning() const {
  return mode_ == Mode::kProvisioning;
}

String WiFiManager::currentSsid() const {
  return mode_ == Mode::kProvisioning ? String(kProvisioningSsid) : ssid_;
}

bool WiFiManager::scanNetworks(String& response) {
  if (!isProvisioning()) {
    return false;
  }

  const int networkCount = WiFi.scanNetworks();
  if (networkCount < 0) {
    return false;
  }

  response = "[";
  for (int index = 0; index < networkCount; ++index) {
    if (index > 0) {
      response += ",";
    }
    response += "{\"ssid\":";
    response += jsonQuoted(String(WiFi.SSID(index)));
    response += ",\"rssi\":";
    response += WiFi.RSSI(index);
    response += "}";
  }
  response += "]";
  return true;
}

bool WiFiManager::saveCredentials(const String& ssid,
                                  const String& password) {
  if (!validateCredentials(ssid, password) ||
      EEPROM.length() < kCredentialsStorageSize) {
    return false;
  }

  EEPROM.put(kMagicAddress, static_cast<uint32_t>(0));
  EEPROM.write(kSsidLengthAddress, static_cast<uint8_t>(ssid.length()));
  EEPROM.write(kPasswordLengthAddress,
               static_cast<uint8_t>(password.length()));

  int address = kCredentialsAddress;
  for (unsigned int index = 0; index < ssid.length(); ++index) {
    EEPROM.write(address++, static_cast<uint8_t>(ssid[index]));
  }
  for (unsigned int index = 0; index < password.length(); ++index) {
    EEPROM.write(address++, static_cast<uint8_t>(password[index]));
  }
  EEPROM.put(kMagicAddress, kCredentialsMagic);

  ssid_ = ssid;
  password_ = password;
  pendingAction_ = PendingAction::kStation;
  return true;
}

void WiFiManager::resetCredentials() {
  if (EEPROM.length() >= kCredentialsStorageSize) {
    for (int address = kMagicAddress; address < kCredentialsStorageSize;
         ++address) {
      EEPROM.write(address, 0);
    }
  }

  ssid_ = "";
  password_ = "";
  connectionReady_ = false;
  pendingAction_ = PendingAction::kProvisioning;
}

bool WiFiManager::loadCredentials() {
  if (EEPROM.length() < kCredentialsStorageSize) {
    Serial.println("EEPROM is too small to store Wi-Fi credentials.");
    return false;
  }

  uint32_t magic = 0;
  EEPROM.get(kMagicAddress, magic);
  if (magic != kCredentialsMagic) {
    return false;
  }

  const uint8_t ssidLength = EEPROM.read(kSsidLengthAddress);
  const uint8_t passwordLength = EEPROM.read(kPasswordLengthAddress);
  if (ssidLength == 0 || ssidLength > 32 || passwordLength > 63) {
    Serial.println("Stored Wi-Fi credentials are invalid.");
    resetCredentials();
    pendingAction_ = PendingAction::kNone;
    return false;
  }

  ssid_ = "";
  password_ = "";
  for (uint8_t index = 0; index < ssidLength; ++index) {
    ssid_ += static_cast<char>(EEPROM.read(kCredentialsAddress + index));
  }
  for (uint8_t index = 0; index < passwordLength; ++index) {
    password_ += static_cast<char>(
        EEPROM.read(kCredentialsAddress + ssidLength + index));
  }

  if (!validateCredentials(ssid_, password_)) {
    Serial.println("Stored Wi-Fi credentials are invalid.");
    resetCredentials();
    pendingAction_ = PendingAction::kNone;
    return false;
  }
  return true;
}

bool WiFiManager::validateCredentials(const String& ssid,
                                      const String& password) const {
  if (ssid.length() == 0 || ssid.length() > 32 || password.length() > 63 ||
      (password.length() > 0 && password.length() < 8)) {
    return false;
  }

  for (unsigned int index = 0; index < ssid.length(); ++index) {
    const uint8_t character = static_cast<uint8_t>(ssid[index]);
    if (character < 0x20 || character == 0x7F) {
      return false;
    }
  }
  for (unsigned int index = 0; index < password.length(); ++index) {
    const uint8_t character = static_cast<uint8_t>(password[index]);
    if (character < 0x20 || character == 0x7F) {
      return false;
    }
  }
  return true;
}

void WiFiManager::startProvisioning() {
  WiFi.disconnect();
  connectionReady_ = false;
  connectionLogged_ = false;
  mode_ = Mode::kProvisioning;
  const int result = WiFi.beginAP(kProvisioningSsid);
  if (result == WL_AP_LISTENING) {
    Serial.print("Provisioning access point started: ");
    Serial.println(kProvisioningSsid);
    Serial.print("Provisioning IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.print("Could not start provisioning access point (status ");
    Serial.print(result);
    Serial.println("). Will retry.");
  }
  lastAttemptMs_ = millis();
  lastStatusCheckMs_ = lastAttemptMs_;
}

void WiFiManager::connect() {
  Serial.println("Connecting to Wi-Fi...");
  const int result = password_.length() == 0
                         ? WiFi.begin(ssid_.c_str())
                         : WiFi.begin(ssid_.c_str(), password_.c_str());
  if (result != WL_CONNECTED) {
    Serial.print("Wi-Fi connection attempt failed (status ");
    Serial.print(result);
    Serial.println("). Will retry.");
  }
  lastAttemptMs_ = millis();
  lastStatusCheckMs_ = lastAttemptMs_;
}
