#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h>

#define CPM_ARR_SIZE 60
#define PERIOD_LOG 1

// Conversion factor - CPM to uSV/h
#define CONV_FACTOR 0.00812

const char * ssid = "";
const char * password = "";

const int inputPin = 2; //input pin from VIN on cajoe shield
volatile unsigned long counts = 0; // Tube events
int cpm = 0; // CPM       
unsigned long lastCountTime; // Time measurement

//Queue data structure to keep last CPM_ARR_SIZE measurements
int cpmArray[CPM_ARR_SIZE];
int cpmArrUsage = 0;

AsyncWebServer httpServer(80); //html server

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("[ INFO ]\tConnecting to wireless network...");
  }
  Serial.println("");
  Serial.println("[ INFO ]\tWiFi connected");
  Serial.print("[ INFO ]\tIP address: ");
  Serial.println(WiFi.localIP());

  if (!SPIFFS.begin(true)) {
    Serial.println("[ ERR  ]\tAn Error has occurred while mounting SPIFFS");
  } else {
    Serial.println("[ INFO ]\tFiles present on local storage:");
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file) {
      Serial.print("[ INFO ]\tFILE: ");
      Serial.println(file.name());
      file = root.openNextFile();
    }
  }

  pinMode(inputPin, INPUT); //input pin of cajoe shield
  attachInterrupt(digitalPinToInterrupt(inputPin), GetEvent, FALLING); //funcition to be called when a pulse is sensed on pin from cajoe shield
  Serial.println("\n[  OK  ]\tGeiger counter started");

  httpServer.begin();

  httpServer.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });
  httpServer.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest * request) {
    request -> send(SPIFFS, "/favicon.png");
  });
  httpServer.on("/main.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request -> send(SPIFFS, "/main.js");
  });

  httpServer.on("/data", HTTP_GET, [](AsyncWebServerRequest * request) {
    request -> send(200, "application/json", "{ \"CPM\": " + String(cpm) + ", \"uSvH\": " + String(cpm * CONV_FACTOR) + "}");
  });

  Serial.println("[  OK  ]\tServer started...");
  Serial.println("[ INFO ]\tSoftware ready. navigate to ip address on port 80...");
}

int value = 0;

void loop() {

  //every second update the queue structure with the counted pulses recived in the last second
  //then, if not enough space is available in the structure, shift the queue and makes space in the last position
  if (millis() - lastCountTime > 1000) {
    int tmp = 0;
    if (cpmArrUsage < CPM_ARR_SIZE) {
      cpmArray[cpmArrUsage] = counts;
      counts = 0;
      cpmArrUsage++;
    } else {
      for (int i = 0; i < CPM_ARR_SIZE - 1; i++) {
        cpmArray[i] = cpmArray[i + 1];
      }
      cpmArray[CPM_ARR_SIZE - 1] = counts;
      counts = 0;
    }
    for (int i = 0; i < cpmArrUsage; i++) {
      tmp += cpmArray[i];
    }
    cpm = tmp;

    lastCountTime = millis();

  }

}

//pulse input handler
ICACHE_RAM_ATTR void GetEvent() {
  counts++;
}
