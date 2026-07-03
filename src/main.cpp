#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

// --- Fill these in ---
const char* WIFI_SSID     = "G";
const char* WIFI_PASSWORD = "12345677";
const char* OTA_PASSWORD  = "esp32ota";

WiFiServer TelnetServer(23);
WiFiClient TelnetClient;

unsigned long lastPrint = 0;
int counter = 0;

// --- Helper functions so printing feels just like Serial.print ---
void tPrint(const String &s) {
  if (TelnetClient && TelnetClient.connected()) TelnetClient.print(s);
}
void tPrintln(const String &s) {
  if (TelnetClient && TelnetClient.connected()) TelnetClient.println(s);
}

void handleTelnetClient() {
  if (TelnetServer.hasClient()) {
    if (!TelnetClient || !TelnetClient.connected()) {
      if (TelnetClient) TelnetClient.stop();
      TelnetClient = TelnetServer.available();
      TelnetClient.println("Connected to ESP32 over WiFi!");
    } else {
      TelnetServer.available().stop(); // reject a 2nd simultaneous client
    }
  }
}

String wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_IDLE_STATUS:     return "IDLE";
    case WL_NO_SSID_AVAIL:   return "NO_SSID_AVAIL (network not found)";
    case WL_CONNECTED:       return "CONNECTED";
    case WL_CONNECT_FAILED:  return "CONNECT_FAILED (wrong password)";
    case WL_CONNECTION_LOST: return "CONNECTION_LOST";
    case WL_DISCONNECTED:    return "DISCONNECTED";
    default:                 return "UNKNOWN (" + String(status) + ")";
  }
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);      // disable WiFi power-save — helps with phone hotspots
  WiFi.disconnect(true);
  delay(500);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 3) {
    Serial.println("Connection attempt " + String(attempts + 1) + " of 3...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(300);
      Serial.print(".");
    }
    Serial.println();
    attempts++;
  }

  Serial.println("Final status: " + wifiStatusToString(WiFi.status()));

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Copy this IP into platformio.ini, then unplug USB and go wireless.");
  } else {
    Serial.println("WiFi FAILED after 3 attempts. Check hotspot settings / phone approval prompt.");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("=== ESP32 WiFi Setup ===");

  connectToWiFi();

  // --- Start OTA (wireless upload listener) ---
  ArduinoOTA.setHostname("esp32-test");
  ArduinoOTA.setPassword(OTA_PASSWORD);
  ArduinoOTA.begin();

  // --- Start Telnet server (wireless "Serial Monitor") ---
  TelnetServer.begin();
  TelnetServer.setNoDelay(true);
}

void loop() {
  ArduinoOTA.handle();     // must run constantly to catch incoming uploads
  handleTelnetClient();    // manage the Telnet connection

  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    tPrintln("Heartbeat #" + String(counter++) + "  (uptime: " + String(millis()/1000) + "s)");
  }
}