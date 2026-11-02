#pragma once

#include <Arduino.h>

class WiFiManager {
 public:
  void begin();
  void update();
  bool isConnected() const;
  bool isProvisioning() const;
  String currentSsid() const;
  bool scanNetworks(String& response);
  bool saveCredentials(const String& ssid, const String& password);
  void resetCredentials();

 private:
  enum class Mode { kProvisioning, kStation };
  enum class PendingAction { kNone, kProvisioning, kStation };

  bool loadCredentials();
  bool validateCredentials(const String& ssid, const String& password) const;
  void startProvisioning();
  void connect();

  String ssid_;
  String password_;
  unsigned long lastAttemptMs_ = 0;
  unsigned long lastStatusCheckMs_ = 0;
  Mode mode_ = Mode::kProvisioning;
  PendingAction pendingAction_ = PendingAction::kNone;
  bool connectionReady_ = false;
  bool connectionLogged_ = false;
  bool ipWarningLogged_ = false;
};
