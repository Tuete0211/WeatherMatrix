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
const char* ssid = "Duesseldorf";
const char* password = "12599740549016360432";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600);

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
  if (!initializeWifi()) {
    Serial.println("\nNot able to connect to wifi");
    handleError(1);
  }

  // Set Time
  Serial.println("\n##########\nInit Time");
  timeClient.begin();
  timeClient.update();

  // Initialize WeatherData
  weatherInfo = WeatherInfo();

  // Initialize Update Cycle


  // Initialize Display Cycle

  delay(2000);
}

bool initializeWifi() {
  WiFi.begin(ssid, password);
  int cycles = 0;
  while (WiFi.status() != WL_CONNECTED) {
    ledMatrix.drawWave(CRGB(50, 50, 180));
    delay(80);
    ledMatrix.drawWave(CRGB(50, 50, 180));
    delay(80);
    Serial.print("-");
    if (++cycles > 100)
      return false;
  }
  Serial.println(">\nMit dem WLAN verbunden!");
  ledMatrix.fadeToBlack();
  return true;
}

// reset function to reboot microcontroller
void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {
  updateCycle();
  cycle();

  delay(2);
}

void cycle() {
  static int cycleCounter = 0;
  static bool flip = true;

  unsigned long currentMillis = millis();

  // every minute -> update brightness
  static unsigned long previousMillisFiveSeconds = 0;
  if (currentMillis - previousMillisFiveSeconds >= 5000) {
    previousMillisFiveSeconds = currentMillis;

    Serial.print("\n-->CycleCounter: ");
    Serial.println(cycleCounter);

    if (cycleCounter == 3) {
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
  Serial.print("hour: ");
  Serial.print(timeClient.getHours());
  Serial.print(", minutes: ");
  Serial.println(timeClient.getMinutes());

  ledMatrix.drawTime(timeClient.getHours(), timeClient.getMinutes());
}


void updateCycle() {
  unsigned long currentMillis = millis();

  // every minute -> update brightness
  static unsigned long previousMillisOneMinute = 0;
  if (currentMillis - previousMillisOneMinute >= 60000) {
    previousMillisOneMinute = currentMillis;

    Serial.println("##UPDATE: \t Brightness");
    //ledMatrix.updateBrightness();
  }

  // every 60 minutes -> update time
  static unsigned long previousMillisOneHour = 0;
  if (currentMillis - previousMillisOneHour >= 60 * 60000) {
    previousMillisOneHour = currentMillis;

    Serial.println("##UPDATE: \t Time");
    timeClient.update();
  }

  // every 180 minutes -> update weather info
  static unsigned long previousMillisThreeHours = 0;
  if (currentMillis - previousMillisThreeHours >= 3 * 60 * 60000) {
    previousMillisThreeHours = currentMillis;

    Serial.println("##UPDATE: \t Weather");
    weatherInfo.updateWeatherData();
  }

  // every 20 hours -> restart
  static unsigned long previousMillisTwentyHours = 0;
  if (currentMillis - previousMillisTwentyHours >= 20 * 60 * 60000) {
    previousMillisTwentyHours = currentMillis;

    Serial.println("##UPDATE: \t Restart");
    resetFunc(); // call reset
  }
}




void handleError(int errorID) {
  if (errorID == 1) {  // red wave
    ledMatrix.drawWave(CRGB(127, 0, 0), true);
    while (true) {
      ledMatrix.drawWave(CRGB(127, 0, 0));
      delay(80);
    }
  }
}
// END OF FILE

//
//
//
//
//
//
//
//
//
//
//
//
//
//      