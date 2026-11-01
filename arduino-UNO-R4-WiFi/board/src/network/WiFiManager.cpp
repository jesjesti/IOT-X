#include "WiFiManager.h"

#include <Arduino.h>
#include <WiFiS3.h>

namespace {

constexpr unsigned long kReconnectIntervalMs = 10000;
constexpr unsigned long kStatusCheckIntervalMs = 1000;

bool hasUsableIp(const IPAddress& address) {
  return address[0] != 0 || address[1] != 0 || address[2] != 0 ||
         address[3] != 0;
}

}  // namespace

void WiFiManager::begin(const char* ssid, const char* password) {
  ssid_ = ssid;
  password_ = password;

  credentialsConfigured_ = ssid_ != nullptr && password_ != nullptr &&
                           ssid_[0] != '\0' && password_[0] != '\0';
  if (!credentialsConfigured_) {
    Serial.println("Wi-Fi credentials missing. Configure include/Secrets.h.");
    return;
  }

  connect();
}

void WiFiManager::update() {
  if (!credentialsConfigured_) {
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

void WiFiManager::connect() {
  Serial.println("Connecting to Wi-Fi...");
  const int result = WiFi.begin(ssid_, password_);
  if (result != WL_CONNECTED) {
    Serial.print("Wi-Fi connection attempt failed (status ");
    Serial.print(result);
    Serial.println("). Will retry.");
  }
  lastAttemptMs_ = millis();
  lastStatusCheckMs_ = lastAttemptMs_;
}
