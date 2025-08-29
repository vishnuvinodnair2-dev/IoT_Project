/*
 * File: Temp_fan
 * Author: Vishnu Vinod Nair
 */
#include "DHT.h"

// DHT22 configuration
#define DHTPIN 4        // GPIO pin connected to DHT22
#define DHTTYPE DHT22   // Specify DHT22 sensor

// Sensors
const int ldrPin = 39;     // ADC pin for LDR
const int sensorPin = 36;  // ADC pin for soil moisture
const int fanPin = 5;

 

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);   // Start serial communication
  dht.begin();          // Initialize the DHT sensor

  pinMode(fanPin, OUTPUT);
}

void loop() {
  readTempHumidity();   // Read and print temperature & humidity
  readMoisture();       // Read and print soil moisture
  readLdr();            // Read and print LDR

  delay(2000);          // Wait 2s before repeating (needed for DHT22!)
}

// Function to read humidity & temperature
void readTempHumidity() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error: Unable to read from DHT sensor.");
    return;
  }

  if (temperature > 27){
    digitalWrite(fanPin, HIGH); // Turn fan ON
  }
  else{
    digitalWrite(fanPin, LOW); // Turn fan OFF
  }

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");
}

// Function to read soil moisture
int readMoisture() {
  int analogValue = analogRead(sensorPin); // Soil moisture sensor value
  Serial.print("Soil Moisture Analog Value: ");
  Serial.println(analogValue);
  return analogValue;
}

// Function to read LDR
int readLdr() {
  int ldrValue = analogRead(ldrPin);
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  return ldrValue;
}
