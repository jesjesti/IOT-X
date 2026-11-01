#pragma once

class WiFiManager {
 public:
  void begin(const char* ssid, const char* password);
  void update();
  bool isConnected() const;

 private:
  void connect();

  const char* ssid_ = nullptr;
  const char* password_ = nullptr;
  unsigned long lastAttemptMs_ = 0;
  unsigned long lastStatusCheckMs_ = 0;
  bool credentialsConfigured_ = false;
  bool connectionReady_ = false;
  bool connectionLogged_ = false;
  bool ipWarningLogged_ = false;
};
