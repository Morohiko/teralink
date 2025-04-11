#include <SPI.h>
#include <LoRa.h>

#define WITH_SERIAL

#ifdef WITH_SERIAL
#define println(str) Serial.println(String("rep: ") + str)
#else
#define println(str)
#endif

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
#define LORA_RX_RST   27
#define LORA_RX_DIO0  25
#define LORA_RX_FREQUENCY 433E6

#define PING_INTERVAL 3000 /* ms */

SPIClass spi_tx = SPIClass(VSPI);
LoRaClass lora_tx;

SPIClass spi_rx = SPIClass(HSPI);
LoRaClass lora_rx;

unsigned long ping_counter = 0u;

void setup() {
#ifdef WITH_SERIAL
  Serial.begin(115200);
#endif
  delay(1000);

  println("LoRa Repeater");

  spi_tx.begin(LORA_TX_SCK, LORA_TX_MISO, LORA_TX_MOSI);
  lora_tx.setSPI(spi_tx);
  lora_tx.setPins(LORA_TX_SS, LORA_TX_RST, LORA_TX_DIO0);
  lora_tx.setPreambleLength(8);

  spi_rx.begin(LORA_RX_SCK, LORA_RX_MISO, LORA_RX_MOSI);
  lora_rx.setSPI(spi_rx);
  lora_rx.setPins(LORA_RX_SS, LORA_RX_RST, LORA_RX_DIO0);

  while (!lora_tx.begin(LORA_TX_FREQUENCY)) {
    println("Starting TX LoRa failed!");
    delay(1000);
  }

  while (!lora_rx.begin(LORA_RX_FREQUENCY)) {
    println("Starting RX LoRa failed!");
    delay(1000);
  }

  lora_rx.setPreambleLength(8);

  println("LoRa Initialization Successful!");

  ping_counter = millis();
}

void loop() {
  int packetSize = lora_rx.parsePacket();
  if (packetSize) {
    while (lora_rx.available()) {
      String received = lora_rx.readString();
      println(received);
      lora_tx.beginPacket();
      lora_tx.print(received);
      lora_tx.endPacket();

      // reset ping counter
      ping_counter = millis();
    }
  }

  if ((millis() - ping_counter) > PING_INTERVAL) {
    // there are no any messages too long, send "ping"
    lora_tx.beginPacket();
    lora_tx.print("ping");
    lora_tx.endPacket();

    // restart lora
    lora_rx.end();
    while (!lora_rx.begin(LORA_RX_FREQUENCY)) {
      println("Starting RX LoRa failed!");
      delay(1000);
    }
    ping_counter = millis();
  }
}
