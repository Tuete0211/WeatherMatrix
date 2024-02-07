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
    //wifiClient = WiFiClientSecure();
    wifiClient.setInsecure();
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
      //*/
      // get data from server
      Serial.println("\n##########\nGet Weather Data");
      if (sender.begin(wifiClient, "https://dataservice.accuweather.com/forecasts/v1/daily/5day/170369?apikey=qvEnALdqsjWRvqVbpOoL7eBOxeGaUBAC&language=de-de&metric=true")) {
        Serial.print("HttpClient started ");
        int httpCode = sender.GET();
        Serial.print("-> Request send ");
        if (httpCode > 0) {
          Serial.print("-> Response available ");
          if (httpCode == 200) {
            Serial.print("-> Good Response ");
            String payload = sender.getString();
            Serial.print("-> save to JSOn Object ");
            DeserializationError error = deserializeJson(weatherData, payload);
            if (error) {
              Serial.print("-> JSON Error: ");
              Serial.print(error.c_str());
              return;
            }
          } else {
            // Falls HTTP-Error
            Serial.print("-> Bad Response (HTTP Code): " + String(httpCode));
          }
        }
        sender.end();
        Serial.println("\nAPI call executed");

      } else {
        Serial.println("HTTP-Verbindung konnte nicht hergestellt werden!");
      }
      //*/
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
        case 15:
          f.iconReference = &iconStorage.thunderstorm;
          break;
        case 18:
        case 26:
          f.iconReference = &iconStorage.rain;
          break;
        default:
          f.iconReference = &iconStorage.ice;
      }

      forecasts[i] = f;
    }
  }

  HTTPClient sender;
  WiFiClientSecure wifiClient;

  // forecasts
  forecast forecasts[3];
};


#endif

// END OF FILE