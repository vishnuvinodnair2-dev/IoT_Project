/*
 * File:  Data_Pub Test
 * Author: Vishnu Vinod Nair
 */
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <DHT.h>
#include <ArduinoJson.h>  // For JSON formatting

// ------------------- Sensor Config -------------------
#define DHTPIN 4         // GPIO pin connected to DHT22
#define DHTTYPE DHT22    // Specify DHT22 sensor 
#define MOISTURE_PIN 36  // ADC pin for soil moisture sensor

// ------------------- WiFi Config ---------------------
const char* ssid = "MonkPc";
const char* password = "12345678";

// ------------------- MQTT Config ---------------------
const char* mqtt_server = "192.168.137.129";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;

DHT dht(DHTPIN, DHTTYPE);

// ------------------- Setup ---------------------------
void setup() {
  Serial.begin(115200);
  dht.begin();  // Initialize DHT22

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(MOISTURE_PIN, INPUT);
}

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

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// ------------------- MQTT Callback -------------------
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);

  // Example: control GPIO via MQTT
  if (String(topic) == "esp32/output") {
    if (messageTemp == "on") {
      Serial.println("Output -> ON");
    } else if (messageTemp == "off") {
      Serial.println("Output -> OFF");
    }
  }
}

// ------------------- Reconnect -----------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 sec");
      delay(5000);
    }
  }
}

// ------------------- Loop ----------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    // Read sensors
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    int moistureRaw = analogRead(MOISTURE_PIN);

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Create JSON object
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["moisture"] = moistureRaw;
    doc["status"] = "ok";

    // Serialize JSON to string
    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    Serial.print("Publishing JSON: ");
    Serial.println(jsonBuffer);

    // Publish JSON
    client.publish("esp32/data", jsonBuffer);
  }
}