/*
 * File:   Final Project Code
 * Author: Vishnu vinod Nair
 */
#include <WiFi.h>
#include <Wire.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ------------------- Config -------------------
bool DEBUG = true;

// Sensors
const int PIN_DHT_SENSOR = 4;
const int DHT_SENSOR_TYPE = DHT22;
const int PIN_LDR_SENSOR = 39;
const int PIN_SOIL_MOISTURE = 36;


// Actuators
const int PIN_COOLING_RELAY = 5;
const int PIN_GROW_LIGHT = 23;
const int GROW_LIGHT_PWM_FREQ = 15000;
const int GROW_LIGHT_PWM_RES = 8;
int growLightCurrentValue = 255;
const int GROW_LIGHT_BRIGHTNESS_MAX = 1400;
const int GROW_LIGHT_BRIGHTNESS_MIN = 600;


// Serial
const long SERIAL_BAUD_RATE = 115200;

// WiFi
const char* WIFI_SSID = "MonkPc";
const char* WIFI_PWD = "12345678";

// MQTT
const char* MQTT_BROKER_IP = "192.168.137.129";
const char* PUBLISH_TOPIC = "esp32/data";
WiFiClient espClient;
PubSubClient client(espClient);


DHT dht(PIN_DHT_SENSOR, DHT_SENSOR_TYPE);

long lastMsg = 0;


// ------------------- Setup -------------------
void setup() {
  // --- Serial Communication ---
  Serial.begin(SERIAL_BAUD_RATE);

  // Init Wifi and MQTT coms
  setup_wifi();

  client.setServer(MQTT_BROKER_IP, 1883);
  client.setCallback(callback);

  // --- Initialize Sensors ---
  dht.begin();

  pinMode(PIN_SOIL_MOISTURE, INPUT);
  pinMode(PIN_COOLING_RELAY, OUTPUT);
  ledcAttach(PIN_GROW_LIGHT, GROW_LIGHT_PWM_FREQ, GROW_LIGHT_PWM_RES);
}

void setup_wifi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("Client IP address: ");
  Serial.println(WiFi.localIP());
}


// Function to return temp and humidity
void readTempHumidity(float &temperature, float &humidity) {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    if (DEBUG) Serial.println("Error: Unable to read from DHT sensor.");
    humidity = -1;
    temperature = -1;
  }
}

// Function to read soil moisture
int readSoilMoisture() {
  int value = analogRead(PIN_SOIL_MOISTURE);
  return value;
}

// Function to read LDR
int readLdr() {
  int value = analogRead(PIN_LDR_SENSOR);  
  return value;
}

// MQTT Callback
void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }

  // Debug output
  if (DEBUG) {
    Serial.print("MQTT Message arrived [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(messageTemp);
  }

}

// Reconnect
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

// Loop
void loop() {

  // MQTT Connect
  if (!client.connected()) reconnect();
  client.loop();

  // Code to run after every 5 seconds
  long now = millis();
  if (now - lastMsg > 5000) {  
    lastMsg = now;

    // --- Read Sensor Values ---
    float temperature = 0, humidity = 0;
    readTempHumidity(temperature, humidity);
    int soilMoisture = readSoilMoisture();
    int ldrValue = readLdr();

    // --- Debug Output ---
    if (DEBUG) {
      Serial.print("Temperature: "); Serial.print(temperature); Serial.print(" °C\t");
      Serial.print("Humidity: "); Serial.print(humidity); Serial.println(" %");
      Serial.print("Soil Moisture: "); Serial.println(soilMoisture);
      Serial.print("LDR Value: "); Serial.println(ldrValue);
      Serial.print("Fan State: "); Serial.println(digitalRead(PIN_COOLING_RELAY) ? "ON" : "OFF");
      Serial.print("Grow Light PWM: "); Serial.println(growLightCurrentValue);
      Serial.println("---------------------------");
    }

    // --- Fan Control ---
    if (temperature > 27) digitalWrite(PIN_COOLING_RELAY, HIGH);
    else digitalWrite(PIN_COOLING_RELAY, LOW);

    // --- Grow Light Control ---
    if (ldrValue < GROW_LIGHT_BRIGHTNESS_MIN) {
      growLightCurrentValue += 50;
      if (growLightCurrentValue > 255) growLightCurrentValue = 255;
    }
    else if (ldrValue > GROW_LIGHT_BRIGHTNESS_MAX) {
      growLightCurrentValue -= 50;
      if (growLightCurrentValue < 0) growLightCurrentValue = 0;
    }
    // Apply PWM
    ledcWrite(PIN_GROW_LIGHT, growLightCurrentValue);

    // --- Create JSON ---
    StaticJsonDocument<200> doc;
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
    doc["moisture"] = soilMoisture;
    doc["ldr"] = ldrValue;
    doc["status"] = "ok";

    // Serialize JSON to string
    char jsonBuffer[256];
    serializeJson(doc, jsonBuffer);

    if (DEBUG) {
      Serial.print("Publishing JSON: "); Serial.println(jsonBuffer);
    }

    // Publish JSON
    client.publish(PUBLISH_TOPIC, jsonBuffer);
  }
}