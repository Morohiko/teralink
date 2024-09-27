#include <nmeaparser.h>
#include <SPI.h>
#include <LoRa.h>

// LoRa pins
#define SCK     18
#define MISO    19
#define MOSI    23
#define SS      5   // NSS
#define RST     12
#define DIO0    14

NMEAParser nmeaParser;

#define SLEEP_BETWEEN_GNSS 1000

#define GNSS_RXD 25
#define GNSS_TXD 26

#define BUFFER_SIZE 256
char inputBuffer[BUFFER_SIZE] = {0u};
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for Serial to be ready
  }
  Serial.println("nmeaparser");
  Serial1.begin(115200, SERIAL_8N1, GNSS_RXD, GNSS_TXD);
  
  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) { // Set frequency to 433 MHz (adjust if necessary)
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initializing OK!");

  delay(1000);
}

void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    String to_send = "";
    if (c != '\n' && c != '\r') {
      // Add character to buffer if space permits
      if (bufferIndex < (BUFFER_SIZE - 1)) {
        inputBuffer[bufferIndex++] = c;
      }
      else {
        // Buffer overflow, reset
        Serial.println("Buffer Overflow. Resetting buffer.");
        bufferIndex = 0;
      }
      continue;
    }

    if (bufferIndex <= 0) 
      continue;

    inputBuffer[bufferIndex] = '\0';
    String sentence = String(inputBuffer);
    bufferIndex = 0;

    sentence.trim();

    // Parse the sentence
    if (!nmeaParser.parseSentence(sentence)) {
            Serial.println("Failed to parse the sentence, not implemented");
      delay(SLEEP_BETWEEN_GNSS);
      continue;
    }

    // Handle GGA data
    if (nmeaParser.isGGAParsed()) {
      GGAData gga = nmeaParser.getGGAData();
      to_send += "utc: " + gga.utcTime;
      to_send += ", lat/lon: " + String(gga.latitude, 10) + "," + String(gga.longitude, 10);
      to_send += ", sats: " + String(gga.numSatellites);
    }

    // Handle GSV data
    if (nmeaParser.isGSVParsed()) {
      GSVData gsv = nmeaParser.getGSVData();
      to_send += ", sats_in_view: " + String(gsv.satellitesInView);
    }
    Serial.println(to_send);

    LoRa.beginPacket();
    LoRa.print(to_send);
    LoRa.endPacket();

    delay(SLEEP_BETWEEN_GNSS);
  }
}
