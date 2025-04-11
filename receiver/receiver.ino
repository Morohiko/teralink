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
#define LORA_RST   D2
#define LORA_DIO0  D1

#define LORA_FREQUENCY 868E6

// Ping
#define PING_TIMEPOUT 4000u

// html
#define HTML_BACKGROUND_COLOR_FAIL "style=\"background-color:red;\""
#define HTML_BACKGROUND_COLOR_SUCCESS "style=\"background-color:green;\""

// Storage
#define STORAGE_SIZE 30u

// Wifi creds
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

AsyncWebServer server(80);

String incomingPackages = "";

class Storage {
public:
  void push(String msg) {
    head = inc(head);
    incomingPackages[head] = msg;
  }

  String getData() {
    int i = inc(head);
    String data = "";
    while (i != head) {
      data += incomingPackages[i];
      i = inc(i);
    }
    return data;
  }

private:
  int head = 0;
  String incomingPackages[STORAGE_SIZE];

  inline int inc(int i) {
    return (++i >= STORAGE_SIZE) ? 0 : i;
  }
};

unsigned long ping_timeout = 0u;
bool is_ping_timeout = false;

Storage storage;

void print_html(AsyncWebServerRequest *request) {
  String to_send = "<!DOCTYPE html><html>"
      "<head><meta charset=\"utf-8\"><title>ESP8266 AP</title></head>"
      "<body " +
      String(is_ping_timeout ? HTML_BACKGROUND_COLOR_FAIL : HTML_BACKGROUND_COLOR_SUCCESS)
      + "><h1>received:</h1></p>" +
      storage.getData() +
      "</body></html>";

  AsyncWebServerResponse *response = request->beginResponse(200, "text/html",
    to_send.c_str()
  );

  response->addHeader("Content-type", "text/html");
  response->addHeader("Connection", "close");
  request->send(response);
}


void onReceive(int packetSize) {
  if (packetSize == 0)
    return;
  String received = LoRa.readString();
  if (!received.equals("ping")) {
    incomingPackages += received;
    incomingPackages += " <br>";
  }
  Serial.println(received);

  ping_timeout = millis();
  is_ping_timeout = false;
}

void setup() {
  Serial.begin(115200);

  delay(1000);

  Serial.println("LoRa Receiver");

  LoRa.setPins(LORA_CS, LORA_RST, LORA_DIO0);

  while (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    delay(1000);
  }
  Serial.println("LoRa Initialization Successful!");

  LoRa.setPreambleLength(8);

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

  ping_timeout = millis();
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    Serial.print("Received packet: ");

    // reset ping timeout
    ping_timeout = millis();
    is_ping_timeout = false;

    while (LoRa.available()) {
      String received = LoRa.readString();
      if (!received.equals("ping")) {
        storage.push(received + " <br>");
      }
      Serial.print(received);
    }

    Serial.print(" with RSSI: ");
    Serial.println(LoRa.packetRssi());
  }

  if (millis() - ping_timeout > PING_TIMEPOUT) {
    is_ping_timeout = true;
    ping_timeout = millis();

    // restart lora
    LoRa.end();
    while (!LoRa.begin(LORA_FREQUENCY)) {
      Serial.println("Starting LoRa failed!");
      delay(1000);
    }
  }
}
