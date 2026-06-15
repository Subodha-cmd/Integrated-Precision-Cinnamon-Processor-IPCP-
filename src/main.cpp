#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>  
#include <PubSubClient.h>

// --- Configuration ---
const char* ssid = "Galaxy A15 5G 675D";
const char* password = "12345677";

// HiveMQ Cluster URL (do not include "mqtt://" or ports, just the address)
const char* mqtt_server = "9ce40a1f20ae4e5da0c2940b2d57cb53.s1.eu.hivemq.cloud"; 
const int mqtt_port = 8883; // 8883 is standard for secure SSL connections
const char* mqtt_user = "suboda";
const char* mqtt_password = "moMO1299do@";

// MQTT Topics
const char* subscribe_topic = "esp32/output";
const char* publish_topic = "esp32/status";

const int ledPin = 2; // GPIO Pin for the onboard LED

WiFiClientSecure espClient; // Use WiFiClientSecure for HiveMQ Cloud (SSL)
PubSubClient client(espClient);

// --- Functions ---

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// This function executes when a message arrives from the broker
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // If a message is received on the topic, check if the message is "on" or "off"
  if (String(topic) == subscribe_topic) {
    if (messageTemp == "on") {
      Serial.println("Turning LED ON");
      digitalWrite(ledPin, HIGH);
      client.publish(publish_topic, "LED is now ON");
    } else if (messageTemp == "off") {
      Serial.println("Turning LED OFF");
      digitalWrite(ledPin, LOW);
      client.publish(publish_topic, "LED is now OFF");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    // Create a unique client ID using a random number to avoid broker conflicts
    String clientId = "ESP32Client-";
    clientId += String(random(0, 0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to the control topic
      client.subscribe(subscribe_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  
  setup_wifi();
  
  // HiveMQ Cloud requires a secure connection. 
  // espClient.setInsecure() skips checking the root certificate chain for testing simplicity.
  espClient.setInsecure(); 
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Keeps the MQTT connection alive and processes incoming packets
}