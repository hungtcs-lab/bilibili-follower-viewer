#include <SH1106.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "Private TCS";
const char* passwd = "hungpass";
const char* userId = "25043249";

uint8_t count = 0;
uint8_t refreshing = 0;

Ticker ticker;
SH1106 display(0x3C, 2, 0);
HTTPClient http;

void showMessage(String message = "loading") {
  display.clear();
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(64, 18, message);
  display.display();
}

boolean isWIFIConnected() {
  return !(WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_CONNECTION_LOST || WiFi.status() == WL_DISCONNECTED);
}

void wifiConnect(const char* ssid, const char* passwd) {
  WiFi.begin(ssid, passwd);
  Serial.printf("Connecting to %s\n", ssid);
  showMessage("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("Connected to %s\n", ssid);
}

uint32_t requestBNumber(const char* userId) {
  int httpCode;
  http.begin("api.bilibili.com", 80, "/x/relation/stat?vmid=" + String(userId));
  httpCode = http.GET();
  if(httpCode == HTTP_CODE_OK) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http.getString());
    JsonObject response = doc.as<JsonObject>();
    return response["data"]["follower"].as<uint32_t>();
  } else {
    Serial.println(http.getString());
    delay(3000);
    Serial.printf("start retry....");
    if(!isWIFIConnected()) {
      wifiConnect(ssid, passwd);
    }
    return requestBNumber(userId);
  }
}

void refreshBNumber() {
  uint32_t number = requestBNumber(userId);
  showMessage(String(number));
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  ticker.attach(1, []() {
    count ++;  
  });
  
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  WiFi.mode(WIFI_STA);
  wifiConnect(ssid, passwd);
  refreshBNumber();
}

void loop() {
  while(!isWIFIConnected()) {
    Serial.printf("Reconnecting...\n");
    wifiConnect(ssid, passwd);
  }

  if(count % 30 == 0) {
    if(refreshing != count) {
      refreshing = count;
      refreshBNumber();
    }
  }
}
