/**
   WeatherMatrix

   @author Fiete Hiersig

   A 10x10 LED Matrix enables displaying weather forecast and the time.

   For this you'll need:
   - ESP32 / NodeMCU v3
   - Button
   - LED-Strip
   - 5V-Powersource

   Used Libraries:
   - FastLED  -> https://github.com/FastLED/FastLED
*/

#define DEV true

#include "LEDMatrix.h"
#include "WeatherInfo.h"

#include <ESP8266WiFi.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

//pin config
#define MODE_PIN 10  // S3

LEDMatrix ledMatrix;
WeatherInfo weatherInfo;

// WLAN-Daten
const char* ssid = "<SSID>";
const char* password = "<PASSWORD>";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200);

int dayCounter = 0;
int screenCounter = 0;

/**
 * Startup sequence:
 * Initialize Serial Connection
 * Initialize LED-Matrix
 * Initialize Wifi
 * Set Time
 * Initialize WeatherData
 * Initialize Update Cycle
 * Initialize Display Cycle
 */
void setup() {
  delay(2000);

  // Initialize Serial Connection
  Serial.begin(115200);

  Serial.println("\n\nVVVVVVVVVVVVVVV\n\n WeatherMatrix\n_______________");
  Serial.println("\nFiete Hiersig\nfiete-hiersig.de\n2024\n_ _ _ _ _ _ _ _\n\n");

  // Initialize LED-Matrix
  Serial.println("1. Init LED-Matrix");
  ledMatrix = LEDMatrix();

  // Initialize Wifi
  Serial.print("\n##########\nInit WiFi\n<");
  //if (!DEV) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      // TODO: visual feedback via LEDMatrix
      delay(200);
      Serial.print("-");
    }
    Serial.println(">\nMit dem WLAN verbunden!");
  //}
  
  // Set Time
  Serial.println("\n##########\nInit Time");
  timeClient.begin();
  timeClient.update();
  
  // Initialize WeatherData
  weatherInfo = WeatherInfo();

  // Initialize Update Cycle


  // Initialize Display Cycle
}

void loop() {

  cycle();
  delay(5000);
}

int cycleCounter = 0;
bool flip = true;
void cycle() {
  Serial.print("\n-->CycleCounter: ");
  Serial.println(cycleCounter);

  if(cycleCounter == 3){
    displayTime();
    cycleCounter = 0;
    return;
  }

  if (flip) {
    displayWeather(cycleCounter);
    flip = !flip;
  } else {
    displayTemperatures(cycleCounter);
    flip = !flip;
    cycleCounter++;
  }
}

void displayWeather(int daysFromNow) {
  Serial.print("display Weather - (icon): ");
  forecast f = weatherInfo.getForecast(daysFromNow);
  Serial.println(f.iconID);

  ledMatrix.drawIcon(f.iconReference, daysFromNow);
}

void displayTemperatures(int daysFromNow) {
  Serial.print("display Temperatures - (max): ");
  forecast f = weatherInfo.getForecast(daysFromNow);
  Serial.print(f.maximumTemperature);
  Serial.print(", (min): ");
  Serial.print(f.minimumTemperature);

  ledMatrix.drawTemperatures(f.maximumTemperature, f.minimumTemperature, daysFromNow);
}

void displayTime() {
  Serial.println("Current time:");
  Serial.println(timeClient.getFormattedTime());
  Serial.print("hour: ");Serial.print(timeClient.getHours());Serial.print(", minutes: ");Serial.println(timeClient.getMinutes());

  ledMatrix.drawTime(timeClient.getHours(), timeClient.getMinutes());
}

// END OF FILE