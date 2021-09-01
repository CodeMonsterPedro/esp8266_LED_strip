#include <ESPAsyncWebServer.h>
#include <WebResponseImpl.h>
#include <AsyncWebSynchronization.h>
#include <AsyncJson.h>
#include <SPIFFSEditor.h>
#include <WebHandlerImpl.h>
#include <AsyncWebSocket.h>
#include <AsyncEventSource.h>
#include <StringArray.h>
#include <WebAuthentication.h>

#include <ESPAsyncTCPbuffer.h>
#include <ESPAsyncTCP.h>
#include <async_config.h>
#include <tcp_axtls.h>
#include <SyncClient.h>
#include <DebugPrintMacros.h>
#include <AsyncPrinter.h>

#include <WiFiClient.h>
#include <BearSSLHelpers.h>
#include <WiFiUdp.h>
#include <ESP8266WiFiGeneric.h>
#include <WiFiServer.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiServerSecureBearSSL.h>
#include <ArduinoWiFiServer.h>
#include <WiFiServerSecure.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiType.h>
#include <ESP8266WiFiGratuitous.h>
#include <CertStoreBearSSL.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiSTA.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266WiFiAP.h>

#include <Arduino.h>
#include <FastLED.h>
#include <string>
#include <EEPROM.h> 

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13

#define SSID "wifi_name" //your WiFi Name
#define PASSWORD "password"  //Your Wifi Password                                                                             

#define PIN D6
#define LEDS_COUNT 10
#define MAX_AMPERS 2000
#define COLOR_ORDER GRB

AsyncWebServer server(80);

int m_brightness = 200;
int m_r = 120;
int m_g = 120;
int m_b = 120;

CRGB leds[LEDS_COUNT];


void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  downloadStartData();
  
  initiateWifiServer();

  FastLED.addLeds<WS2812B, PIN, COLOR_ORDER>(leds, LEDS_COUNT)/*.setCorrection( TypicalLEDStrip )*/;
  //if (CURRENT_LIMIT > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, CURRENT_LIMIT);
  FastLED.clear();
  FastLED.setBrightness(m_brightness);
  setColor(m_r, m_g, m_b);
  FastLED.show(); 
}

void loop() {
  FastLED.show();
}

void uploadStartData(){
  Serial.println("uploadStartData");
  int offset = 0;
  EEPROM.put(offset, m_brightness);
  EEPROM.commit();
  offset += sizeof(int);
  EEPROM.put(offset, m_r);
  EEPROM.commit();
  offset += sizeof(int);
  EEPROM.put(offset, m_g);
  EEPROM.commit();
  offset += sizeof(int);
  EEPROM.put(offset, m_b);
  EEPROM.commit();
}

void downloadStartData(){
  Serial.println("downloadStartData");
  int offset = 0;
  EEPROM.get(offset, m_brightness);
  offset += sizeof(int);
  EEPROM.get(offset, m_r);
  offset += sizeof(int);
  EEPROM.get(offset, m_g);
  offset += sizeof(int);
  EEPROM.get(offset, m_b);
}

std::string updateMenuPage(){
  std::string s = "<!DOCTYPE HTML><html>\n<br><br><form action=\"/get\">\n Brightness (0-240): <input name=\"BRIGHTNESS\" value=\"";
  s += std::to_string(m_brightness);
  s += "\"><br>\n <br><input type=\"submit\" value=\"Set\">\n</form><br><br> <form action=\"/get\">\n R (0-255): <input name=\"R\" value=\"";
  s += std::to_string(m_r);
  s += "\"><br>\n G (0-255): <input name=\"G\" value=\"";
  s += std::to_string(m_g);
  s += "\"><br>\n B (0-255): <input name=\"B\" value=\"";
  s += std::to_string(m_b);
  s += "\"><br>\n<br> <input type=\"submit\" value=\"Set\">\n </form></html>";
  return s;
}

void setBrightnessLevel(int brightness){
  if (brightness > 240 || brightness < 0) return;
  m_brightness = brightness;
  Serial.println("setBrightnessLevel");
  Serial.print(m_brightness);
  Serial.println("");
  FastLED.setBrightness(m_brightness);
  FastLED.show();

  uploadStartData();
}

void setColor(int r, int g, int b){
  if (r > 255 || r < 0) return;
  if (g > 255 || g < 0) return;
  if (b > 255 || b < 0) return;
  
  m_r = r;
  m_g = g;
  m_b = b;

  Serial.println("setColor");
  Serial.print(m_r);
  Serial.print(":");
  Serial.print(m_g);
  Serial.print(":");
  Serial.print(m_b);
  Serial.println("");

  for (int i = 0; i < LEDS_COUNT; ++i){
    leds[i] = CRGB(m_r,m_g,m_b);
  }
  FastLED.show();

  uploadStartData();
}

void initiateWifiServer(){
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", updateMenuPage().c_str());
  });

  server.on("/get", HTTP_GET, [&m_brightness, &m_r, &m_g, &m_b] (AsyncWebServerRequest *request) {
    if (request->hasParam("BRIGHTNESS")) {
      setBrightnessLevel(request->getParam("BRIGHTNESS")->value().toInt());
    }

    if (request->hasParam("R")) {
      setColor(request->getParam("R")->value().toInt(), request->getParam("G")->value().toInt(), request->getParam("B")->value().toInt());
    }
    request->send_P(200, "text/html", updateMenuPage().c_str());
  });
  server.begin();
}
