#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

class NetHelper {
  public:
    // Call once in setup(). Connects to WiFi, starts OTA, starts Telnet server,
    // and spins up the background task on Core 0 automatically.
    void begin(const char* ssid,
               const char* password,
               const char* hostname,
               const char* otaPassword,
               uint16_t telnetPort = 23);

    // Send text to the connected Telnet client (acts like Serial.print/println)
    void print(const String &msg);
    void println(const String &msg);

    // Read incoming text from the connected Telnet client (acts like Serial.available/read)
    int  available();
    char read();

    bool isConnected();
    IPAddress getIP();

  private:
    WiFiServer* _telnetServer = nullptr;
    WiFiClient  _telnetClient;
    uint16_t    _telnetPort   = 23;

    void connectWiFi(const char* ssid, const char* password);
    void handleTelnetClient();
    void loopOnce();               // one pass of OTA + Telnet handling

    void startBackgroundTask();
    static void taskFunc(void* param); // FreeRTOS task entry point
};