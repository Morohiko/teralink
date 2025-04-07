#include <SPI.h>
#include <LoRa.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebSrv.h>

// LORA SPI pins
#define LORA_SCK   D5
#define LORA_MISO  D6
#define LORA_MOSI  D7
#define LORA_CS    D8
#define LORA_RST   D1
#define LORA_DIO0  D0

#define LORA_FREQUENCY 868E6

// Wifi creds
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

AsyncWebServer server(80);

String incomingPackages = "";

void print_html(AsyncWebServerRequest *request) {
  String to_send = "<!DOCTYPE html><html>"
      "<head><meta charset=\"utf-8\"><title>ESP8266 AP</title></head>"
      "<body><h1>received:</h1></p>" +
      incomingPackages +
      "</body></html>";

  AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
    to_send.c_str()
  );

  response->addHeader("Content-type", "text/html");
  response->addHeader("Connection", "close");
  request->send(response);
}

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("LoRa Receiver");

  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  while (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
  }
  Serial.println("LoRa Initialization Successful!");

  LoRa.receive();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", [](AsyncWebServerRequest *request){
    Serial.println("request");

    print_html(request);
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: ");

    while (LoRa.available()) {
      String received = LoRa.readString();
      incomingPackages += received;
      incomingPackages += " <br>";
      Serial.print(received);
    }

    Serial.print(" with RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}
