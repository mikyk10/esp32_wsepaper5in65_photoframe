/**
ESP32_WSEPaper-5in65_PhotoFrame
Copyright (C) 2022 Misutaka Kato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/
#include "config.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>

#include "epd.h"  // e-Paper driver

#define DEFAULT_TIME_TO_SLEEP 43200
#define uS_TO_SEC_FACTOR 1000000ULL
#define HTTP_TIMEOUT     30000
#define BUF_SIZE         512
#define LED              2

// put the system into deepsleep for certain seconds
void deepSleep(int seconds) {
  Serial.println("[sys] Going into deep sleep");
  esp_sleep_enable_timer_wakeup(seconds * uS_TO_SEC_FACTOR);
  esp_deep_sleep_start();
}

// blink the LED with specified interval and turn off the LED
void blinkLED(int count, int interval) {
  digitalWrite(LED, LOW);
  delay(interval);
  
  int state = 1; 
  for (int i=0;i<count*2;i++) {
    digitalWrite(LED, state);
    state = state ^ 1;
    delay(interval);
  }

  digitalWrite(LED, LOW);
}


void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // blink the LED for showing the device is alive
  blinkLED(1, 500);

  // init ePaper
  EPD_initSPI();
  Serial.println("[SPI] init");

  // clear the display 
  digitalWrite(LED, HIGH);
  Serial.println("[disp] Clearing...");
  EPD_5IN65F_init();
  EPD_5IN65F_Clear();
  digitalWrite(LED, LOW);

  // Connect to WiFi
  Serial.print(F("[net] Connecting WiFi: "));
  WiFi.begin(WiFi_SSID, WiFi_PASSWD, 6);
  {
    int c = 0;
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");

      // WiFi timeout
      if (c > 100) {
        Serial.println("timeout");
        WiFi.mode(WIFI_OFF);

        // Long Blink LED 5 times for WiFi failure and go to deepsleep
        blinkLED(5, 1000);

        deepSleep(DEFAULT_TIME_TO_SLEEP);
        return;
      }

      c++;
      delay(100);
    }
  }
  
  //IPAddress serverIp MDNS.queryHost("rpi4-miky.local")
  //Serial.println(serverIp.toString());

  // turn the LED off 
  digitalWrite(LED, LOW);

  Serial.println(" connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  WiFiClient *wifiClient;
  WiFiClient *wifiStream;
  HTTPClient *httpClient;
  uint contentLength;

  wifiClient = new WiFiClient;
  
  // init network access clinets
  httpClient = new HTTPClient();
  httpClient->setTimeout(HTTP_TIMEOUT);
  httpClient->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  Serial.print("[http] begin...\n");

  char shouldRefresh = 0;
  int sleepSeconds   = DEFAULT_TIME_TO_SLEEP;
  
  // send a request to the server
  if (httpClient->begin(*wifiClient, imageURL)) {
    Serial.print("[http] GET... ");
    // start connection and send HTTP header
    int httpCode = httpClient->GET();

    // httpCode will be negative on error
    if (httpCode < 0) {
      Serial.printf("[http] Unable to connect\n");
      // Blink LED 5 times for connection error
      blinkLED(5, 250);

      Serial.printf("[http] GET... failed, error: %s\n", httpClient->errorToString(httpCode).c_str());
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("code: %d\n", httpCode);

      // refresh display content
      if (httpCode == HTTP_CODE_OK) {
        // get length of document (is -1 when Server sends no Content-Length header)
        contentLength = httpClient->getSize();
        Serial.printf("[http] Content-Length: %d\n", contentLength);
      } else {
        // Blink LED 5 times for any non 200 http response 
        blinkLED(5, 250);
      }

      if (httpCode == HTTP_CODE_OK && contentLength > 0) {
        shouldRefresh = 1;

        // get tcp stream
        wifiStream = httpClient->getStreamPtr();
        
        EPD_5IN65F_init();

        Serial.printf("[disp] Transferring data: ");
        
        uint8_t buff[BUF_SIZE];

        while(contentLength > 0 || contentLength == -1) {
          // get available data size
          size_t size = wifiStream->available();

          if(size) {
            // read data stream and send them to the display
            int c = wifiStream->read(buff, BUF_SIZE);
            Serial.printf(".");

            // write it to Serial
            uint8_t *p = buff;

            for (int i = 0; i < c; i++) {
              EPD_SendData(*p);
              p++;
            }

            if(contentLength > 0) {
              contentLength -= c;
            }
          }
          delay(1);
        }
        Serial.println("done.");
      }
    }

    // determine period to sleep by server's instruction or use default
    sleepSeconds = httpClient->header("X-Sleep-Seconds").toInt(); 
    if (sleepSeconds == 0) {
      sleepSeconds = DEFAULT_TIME_TO_SLEEP;
    }

    // turn the LED off
    digitalWrite(LED, LOW);

    httpClient->end();

    // Disable WiFi and the LED and refresh the display
    digitalWrite(LED, LOW);
    WiFi.mode(WIFI_OFF);
    Serial.println("[net] Wi-Fi off");

    // refresh the display
    if (shouldRefresh) {
      digitalWrite(LED, HIGH);
      Serial.println("[disp] refreshing");
      EPD_5IN65F_Show();  
      digitalWrite(LED, LOW);
    }
  }

  delete httpClient;
  delete wifiClient;

  // Put ESP32 into Deep Sleep
  EPD_5IN65F_Sleep();
  deepSleep(sleepSeconds);
}

void loop() {
  // nothing to do
}

