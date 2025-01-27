#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);
Adafruit_BME280 bme;

void setup() {
  Serial.begin(9600);
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  bme.begin(0x76);
}

void loop() {
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;  

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Weather Station Data");

  display.setCursor(0, 16);
  display.print("Temperature: ");
  display.print(temperature);
  display.println(" C");

  display.setCursor(0, 32);
  display.print("Humidity: ");
  display.print(humidity);
  display.println(" %");

  display.setCursor(0, 48);
  display.print("Pressure: ");
  display.print(pressure);
  display.println(" hPa");

  display.display();

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Pressure: ");
  Serial.print(pressure);
  Serial.println(" hPa");

  delay(1000);
}