const int ledPin = 23;  // GPIO pin for LED
const int freq = 15000; // PWM frequency
const int resolution = 8; // 8-bit resolution (0-255)

int ldrPin = 39; // Or any other ADC-enabled GPIO pin
int ldrValue = 0;

// variable for brightness
int brightness = 255;
const int maxbrightness = 1400;
const int minbrightness = 600;

void setup() {
  Serial.begin(9600);

  // configure LED PWM functionalities
  ledcAttach(ledPin, freq, resolution);
}

void loop() {

  ldrValue = analogRead(ldrPin); // Read the analog value from the LDR
  Serial.print("LDR Value: ");
  Serial.println(ldrValue); // Print the value to the serial monitor
  Serial.print("Brightness: ");
  Serial.println(brightness);
  delay(500); // Wait for half a second before the next reading

  if(ldrValue < maxbrightness)
  {
    brightness+=20;

    if(brightness > 255)
    {
      brightness = 255;
    }
    ledcWrite(ledPin, brightness);
  }

  if(ldrValue > minbrightness)
  {
    brightness-=20;

    if(brightness < 0)
    {
      brightness = 0;
    }
    ledcWrite(ledPin, brightness);
  }
}