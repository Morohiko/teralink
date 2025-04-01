#include <SPI.h>
#include <LoRa.h>

#define LORA_TX_SCK   5
#define LORA_TX_MISO  18
#define LORA_TX_MOSI  19
#define LORA_TX_SS    21
#define LORA_TX_RST   32
#define LORA_TX_DIO0  33
#define LORA_TX_FREQUENCY 868E6

#define LORA_RX_SCK   14
#define LORA_RX_MISO  12
#define LORA_RX_MOSI  13
#define LORA_RX_SS    15
#define LORA_RX_RST   33
#define LORA_RX_DIO0  32  
#define LORA_RX_FREQUENCY 433E6

SPIClass spi_tx = SPIClass(VSPI);
LoRaClass lora_tx;

SPIClass spi_rx = SPIClass(HSPI);
LoRaClass lora_rx;

void setup() {
  Serial.begin(115200);

  Serial.println("LoRa Transmitter");

  spi_tx.begin(LORA_TX_SCK, LORA_TX_MISO, LORA_TX_MOSI);
  lora_tx.setSPI(spi_tx);
  lora_tx.setPins(LORA_TX_SS, LORA_TX_RST, LORA_TX_DIO0);

  spi_rx.begin(LORA_RX_SCK, LORA_RX_MISO, LORA_RX_MOSI);
  lora_rx.setSPI(spi_rx);
  lora_rx.setPins(LORA_RX_SS, LORA_RX_RST, LORA_RX_DIO0);

  while (!lora_tx.begin(LORA_TX_FREQUENCY)) {
    Serial.println("Starting TX LoRa failed!");
    delay(1000);
  }

  while (!lora_rx.begin(LORA_RX_FREQUENCY)) {
    Serial.println("Starting RX LoRa failed!");
    delay(1000);
  }

  Serial.println("LoRa Initialization Successful!");

  lora_rx.receive();
}

void loop() {
  int packetSize = lora_rx.parsePacket();
  if (packetSize) {
    Serial.println("Received packet: ");

    while (lora_rx.available()) {
      String received = lora_rx.readString();
      Serial.println(received);
      lora_tx.beginPacket();
      lora_tx.print(received);
      lora_tx.endPacket();
    }

    Serial.println(" with RSSI: ");
  }
}
