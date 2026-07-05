#include "NetHelper.h"

void NetHelper::connectWiFi(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.disconnect(true);
  delay(500);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 3) {
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(300);
    }
    attempts++;
  }
}

void NetHelper::begin(const char* ssid,
                       const char* password,
                       const char* hostname,
                       const char* otaPassword,
                       uint16_t telnetPort) {
  _telnetPort = telnetPort;

  connectWiFi(ssid, password);

  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.setPassword(otaPassword);
  ArduinoOTA.begin();

  _telnetServer = new WiFiServer(_telnetPort);
  _telnetServer->begin();
  _telnetServer->setNoDelay(true);

  startBackgroundTask();   // networking now runs on Core 0, forever, on its own
}

void NetHelper::handleTelnetClient() {
  if (_telnetServer->hasClient()) {
    if (!_telnetClient || !_telnetClient.connected()) {
      if (_telnetClient) _telnetClient.stop();
      _telnetClient = _telnetServer->available();
      _telnetClient.println("Connected to ESP32 over WiFi!");
    } else {
      _telnetServer->available().stop(); // reject a 2nd simultaneous client
    }
  }
}

void NetHelper::loopOnce() {
  ArduinoOTA.handle();
  handleTelnetClient();
}

// --- FreeRTOS background task machinery ---

void NetHelper::taskFunc(void* param) {
  NetHelper* self = static_cast<NetHelper*>(param);
  while (true) {
    self->loopOnce();
    vTaskDelay(1 / portTICK_PERIOD_MS); // yield ~1ms so this task doesn't hog Core 0
  }
}

void NetHelper::startBackgroundTask() {
  xTaskCreatePinnedToCore(
    taskFunc,        // function to run
    "NetHelperTask", // task name (for debugging)
    4096,            // stack size in bytes
    this,            // parameter passed into taskFunc (this NetHelper instance)
    1,               // priority
    NULL,            // no task handle needed
    0                // pin to Core 0 — main sketch loop() runs on Core 1
  );
}

void NetHelper::print(const String &msg) {
  if (_telnetClient && _telnetClient.connected()) _telnetClient.print(msg);
}

void NetHelper::println(const String &msg) {
  if (_telnetClient && _telnetClient.connected()) _telnetClient.println(msg);
}

int NetHelper::available() {
  if (_telnetClient && _telnetClient.connected()) return _telnetClient.available();
  return 0;
}

char NetHelper::read() {
  return _telnetClient.read();
}

bool NetHelper::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress NetHelper::getIP() {
  return WiFi.localIP();
}