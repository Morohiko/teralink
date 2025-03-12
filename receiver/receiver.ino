#include <SPI.h>
#include <LoRa.h>

// Load Wi-Fi library
#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>

// LoRa pins for Arduino Nano
#define ss 10    // NSS
#define rst 9    // RST
#define dio0 2   // DIO0

// Wifi creds
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

// Wifi server port: 80
WiFiServer server(80);

// Consist incoming data to print
String incomingPackages = "";

void print_html(NetworkClient wifiClient) {
  wifiClient.println("HTTP/1.1 200 OK");
  wifiClient.println("Content-type:text/html");
  wifiClient.println("Connection: close");
  wifiClient.println();

  // Display the HTML web page
  wifiClient.println("<!DOCTYPE html> <html>");
  wifiClient.println("<head> <title>gps tracker</title> </head>");
  wifiClient.println("<body> <h1>gps tracker</h1>");
  wifiClient.println(incomingPackages);
  wifiClient.println("</body> </html>");
  wifiClient.println();
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");

  // Lora initialization
  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(512);
  LoRa.setSignalBandwidth(64000);
  LoRa.setCodingRate4(5);

  if (!WiFi.softAP(ssid, password)) {
    Serial.print("Soft AP creation failed.");
    while (1);
  }

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");
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

  NetworkClient client = server.accept();
  if (client && client.connected()) {
    Serial.println("client connected, print..");
    print_html(client);
  }
  client.stop();
}
