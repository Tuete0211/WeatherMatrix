//WeatherInfo.h
#ifndef WEATHERINFO_H
#define WEATHERINFO_H

#include <Arduino.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

struct forecast {  // -> change to struct
  int maximumTemperature = 0;
  int minimumTemperature = 0;
  int iconID = 0;
  const unsigned int (*iconReference)[10][10];
};

class WeatherInfo {
public:
  WeatherInfo() {
    requestWeatherData();
  }

  void updateWeatherData(){
    requestWeatherData();
  }

  void printWeatherData() {
    // Hier lesen wir die Werte über den Buffer aus
    Serial.println("----------\nData:\n");

    for (int i = 0; i < 3; i++) {
      if (i == 0)
        Serial.println("Today");
      if (i == 1)
        Serial.println("Tomorrow");
      if (i == 2)
        Serial.println("Over Tomorrow");
      //Serial.println("Time: " + String(weatherData["DailyForecasts"][i]["Date"]));
      Serial.println("Max: " + String(forecasts[i].maximumTemperature) + ", Min: " + String(forecasts[i].minimumTemperature));
      Serial.println("Icon-Code: " + String(forecasts[i].iconID));  // + ", Desc: " + String(weatherData["DailyForecasts"][i]["Day"]["IconPhrase"]));
      Serial.println();
    }
  }

  forecast getForecast(int daysFromNow) {
    return forecasts[daysFromNow];
  }

private:

  void requestWeatherData() {
    DynamicJsonDocument weatherData(4096);

    if (DEV) {
      // Dummy Data for Testing
      Serial.println("\n##########\nLoad Weather Data");
      String data = "{'DailyForecasts':[{'Date':'2023-04-29T07:00:00+02:00','EpochDate':1682744400,'Temperature':{'Minimum':{'Value':6.8},'Maximum':{'Value':9.2}},'Day':{'Icon':4}},{'Date':'2023-04-30T07:00:00+02:00','EpochDate':1682830800,'Temperature':{'Minimum':{'Value':8.8},'Maximum':{'Value':19.6}},'Day':{'Icon':7}},{'Date':'2023-05-01T07:00:00+02:00','EpochDate':1682917200,'Temperature':{'Minimum':{'Value':15.3},'Maximum':{'Value':33.8}},'Day':{'Icon':5},'MobileLink':'http://www.accuweather.com/de/de/bonn/53111/daily-weather-forecast/170369?day=3&unit=c'}]}";
      DeserializationError error = deserializeJson(weatherData, data);
      if (error) {
        Serial.print("-> JSON Error: ");
        Serial.print(error.c_str());
        return;
      }
    } else {

      WiFiClient client;

      HTTPClient http;

      Serial.print("[HTTP] begin...\n");
      if (http.begin(client, "http://dataservice.accuweather.com/forecasts/v1/daily/5day/170369?apikey=qvEnALdqsjWRvqVbpOoL7eBOxeGaUBAC&language=de-de&metric=true")) {  // HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0) {
          // HTTP header has been send and Server response header has been handled
          Serial.printf("[HTTP] GET... code: %d\n", httpCode);

          // file found at server
          if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            String payload = http.getString();
            Serial.print("-> save to JSOn Object ");
            DeserializationError error = deserializeJson(weatherData, payload);
            if (error) {
              Serial.print("-> JSON Error: ");
              Serial.print(error.c_str());
              return;
            }
            Serial.println(payload);
          }
        } else {
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
      } else {
        Serial.printf("[HTTP} Unable to connect\n");
      }
    }

    // save necessary data for later use
    for (int i = 0; i < 3; i++) {
      forecast f;
      f.maximumTemperature = (int)weatherData["DailyForecasts"][i]["Temperature"]["Maximum"]["Value"];
      f.minimumTemperature = (int)weatherData["DailyForecasts"][i]["Temperature"]["Minimum"]["Value"];
      f.iconID = (int)weatherData["DailyForecasts"][i]["Day"]["Icon"];

      // select corresponding icon from IconStorage
      switch (f.iconID) {
        case 1:
        case 2:
          f.iconReference = &iconStorage.sunny;
          break;
        case 3:
        case 4:
        case 5:
        case 21:
          f.iconReference = &iconStorage.partly_sunny;
          break;
        case 6:
        case 20:
          f.iconReference = &iconStorage.mostly_cloudy;
          break;
        case 7:
        case 19:
          f.iconReference = &iconStorage.cloudy;
          break;
        case 8:
          f.iconReference = &iconStorage.dark_cloudy;
          break;
        case 11:
          f.iconReference = &iconStorage.foggy;
          break;
        case 12:
          f.iconReference = &iconStorage.showers;
          break;
        case 13:
          f.iconReference = &iconStorage.mostly_cloudy_showers;
          break;
        case 14:
          f.iconReference = &iconStorage.partly_sunny_showers;
          break;
        case 15:
        case 16:
        case 17:
          f.iconReference = &iconStorage.thunderstorm;
          break;
        case 18:
        case 26:
          f.iconReference = &iconStorage.rain;
          break;
        case 30:
          f.iconReference = &iconStorage.hot;
          break;
        case 31:
          f.iconReference = &iconStorage.cold;
          break;
        default:
          f.iconReference = &iconStorage.ice;
      }

      forecasts[i] = f;
    }
  }

  // forecasts
  forecast forecasts[3];
};


#endif

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
//
//
//
//
//
//
//
//