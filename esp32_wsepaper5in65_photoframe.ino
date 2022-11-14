#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <HTTPClient.h>


#include "buff.h" // POST request data accumulator
#include "epd.h"  // e-Paper driver

const char* WiFi_SSID   = "KMYD4-1122C4-ISg";
const char* WiFi_PASSWD = "777aaafvx579ctar";


#define uS_TO_SEC_FACTOR 1000000ULL
#define TIME_TO_SLEEP    86400
#define HTTP_TIMEOUT     30000

#define BUF_SIZE         512


//#define HTTP_SECURE

const char* imageURL = "http://192.168.32.207:9002/random.bin";
static WiFiClient *g_WiFiClient;

static WiFiClient *g_WiFiStream;
static HTTPClient *g_HTTPClient;
static uint g_nInFileSize;

/* ------ */
void setup() {
  EPD_initSPI();

  //IPAddress serverIp MDNS.queryHost("rpi4-miky.local")
  //Serial.println(serverIp.toString());

  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.print(F("Connecting WiFi: "));
  WiFi.begin(WiFi_SSID, WiFi_PASSWD, 6);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(" connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // synchronize time with a NTP server
  //configTime(0, 0, "pool.ntp.org");
  //Serial.print(F("Waiting for NTP time sync: "));
  //time_t nowSecs = time(nullptr);
  //while (nowSecs < 8 * 3600 * 2) {
    //delay(500);
    //Serial.print(F("."));
    //yield();
    //nowSecs = time(nullptr);
  //}

  Serial.println();
  //struct tm timeinfo;
  //gmtime_r(&nowSecs, &timeinfo);
  //Serial.print(F("Current time: "));
  //Serial.print(asctime(&timeinfo));
}


// main loop
void loop() {

#ifdef HTTP_SECURE
  g_WiFiClient = new WiFiClientSecure;
  g_WiFiClient->setCACert(rootCACertificate);
  g_WiFiClient->setInsecure();
#else
  g_WiFiClient = new WiFiClient;
#endif

  if(g_WiFiClient) {
    g_HTTPClient = new HTTPClient();
    g_HTTPClient->setTimeout(HTTP_TIMEOUT);
    g_HTTPClient->setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    Serial.print("[HTTP(S)] begin...\n");
    if (g_HTTPClient->begin(*g_WiFiClient, imageURL)) {
      Serial.print("[HTTP(S)] GET... ");
      // start connection and send HTTP header
      int httpCode = g_HTTPClient->GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK) {
          // get length of document (is -1 when Server sends no Content-Length header)
          g_nInFileSize = g_HTTPClient->getSize();
          Serial.printf("[HTTP(S)] Content-Length: %d\n", g_nInFileSize);

          // get tcp stream
          g_WiFiStream = g_HTTPClient->getStreamPtr();
          
          Serial.printf("[Display] Init\n");
          EPD_5IN65F_init();
          Serial.printf("[Display] Transferring data: ");

          uint8_t buff[BUF_SIZE];

          // read all data from server
          while(g_nInFileSize > 0 || g_nInFileSize == -1) {
              // get available data size
              size_t size = g_WiFiStream->available();

              if(size) {
                  // read a byte
                  int c = g_WiFiStream->read(buff, BUF_SIZE);
                  Serial.printf(".");

                  // write it to Serial
                  uint8_t *p = buff;

                  for (int i = 0; i < c; i++) {
                    EPD_SendData(*p);
                    p++;
                  }

                  if(g_nInFileSize > 0) {
                      g_nInFileSize -= c;
                  }
              }
              delay(1);
          }

          Serial.println("done.");

        }
      } else {
        Serial.printf("[HTTP(S)] GET... failed, error: %s\n", g_HTTPClient->errorToString(httpCode).c_str());
      }

      g_HTTPClient->end();

      // Disable WiFi and Refresh
      WiFi.mode(WIFI_OFF);
      EPD_5IN65F_Show();

    } else {
      Serial.printf("[HTTP(S)] Unable to connect\n");
    }

    delete g_HTTPClient;
    delete g_WiFiClient;

  } else {
    Serial.println("Unable to create g_WiFiClient");
  }

  Serial.println("Finished.");

  // Put ESP32 into Deep Sleep
  Serial.println("Going to deep sleep...");
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_SEC_FACTOR);
  esp_deep_sleep_start();
}
