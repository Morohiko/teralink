#include <SPI.h>
#include <LoRa.h>

// LoRa pins for Arduino Nano
#define ss 10    // NSS
#define rst 9    // RST
#define dio0 2   // DIO0

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");

  LoRa.setPins(ss, rst, dio0);

  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(512);
  LoRa.setSignalBandwidth(64000);
  LoRa.setCodingRate4(5);
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: ");

    while (LoRa.available()) {
      String received = LoRa.readString();
      Serial.print(received);
    }

    Serial.print(" with RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
}

