#include <SPI.h>
#include <LoRa.h>

// LoRa pins
#define SCK     18
#define MISO    19
#define MOSI    23
#define SS      5   // NSS
#define RST     12
#define DIO0    14

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Transmitter");

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) { // Set frequency to 433 MHz (adjust if necessary)
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initializing OK!");
}

int packetNumber = 0;

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(packetNumber);

  // Start packet
  LoRa.beginPacket();
  LoRa.print("Hello World ");
  LoRa.print(packetNumber);
  LoRa.endPacket();

  packetNumber++;

  delay(5000); // Wait 5 seconds before sending next packet
}

