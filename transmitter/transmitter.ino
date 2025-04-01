#include <nmeaparser.h>
#include <SPI.h>
#include <LoRa.h>

// LoRa pins
#define SCK     10
#define MISO    11
#define MOSI    12
#define SS      13
#define RST     1
#define DIO0    2

// GNSS pins
#define GNSS_RXD 8
#define GNSS_TXD 7

// buzzer pins
#define BUZZER_PLUS 5
#define BUZZER_GND 6
#define BUZZER_IS_CONNECTED 3

NMEAParser nmeaParser;

#define BUFFER_SIZE 256
char inputBuffer[BUFFER_SIZE] = {0u};
int bufferIndex = 0;

struct gnss_data {
  String utc;
  float lon;
  float lat;
  int sats;
};

struct gnss_data last_data = {
  .utc = String("00:00:00"),
  .lon = 0.0,
  .lat = 0.0,
  .sats = 0
};

unsigned long previousMillis = 0u;
const unsigned long interval_millis = 1000u; // 1 second

void setup() {
  Serial.begin(115200);

  Serial.println("nmeaparser");

  Serial1.begin(115200, SERIAL_8N1, GNSS_RXD, GNSS_TXD);
  
  SPI.begin(SCK, MISO, MOSI, SS);

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    delay(1000u);
  }

  Serial.println("LoRa Initializing OK!");

  previousMillis = millis();

  pinMode(BUZZER_PLUS, OUTPUT);
  pinMode(BUZZER_GND, OUTPUT);
  pinMode(BUZZER_IS_CONNECTED, INPUT);

  // sleep
  delay(1000);
}

void send_lora_message() {
  String to_send = "";
  if (last_data.utc.length() > 0) {
    to_send += "utc: " + last_data.utc + ", ";
  }
  to_send += "sats: " + String(last_data.sats) + ", ";
  to_send += "lat/lon: " + String(last_data.lat, 10) + "," + String(last_data.lon, 10);

  LoRa.beginPacket();
  LoRa.print(to_send);
  LoRa.endPacket();

  Serial.println(to_send);
}

void smart_delay() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval_millis) {
    previousMillis = currentMillis;
    send_lora_message();
  }
}

inline String convert_utc_time(String ggaSentence) {
  return ggaSentence.substring(0, 2) + ":" + ggaSentence.substring(2, 4) + ":" + ggaSentence.substring(4, 6);
}

void loop() {
  while (Serial1.available()) {
    int is_buzzer_connected = digitalRead(BUZZER_IS_CONNECTED);
    // turn on buzzer
    // digitalWrite(BUZZER_PLUS, !is_buzzer_connected);
    // digitalWrite(BUZZER_GND, 0);

    char c = Serial1.read();
    if (c != '\n' && c != '\r') {
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
      continue;
    }

    // Handle GGA data
    if (nmeaParser.isGGAParsed()) {
      GGAData gga = nmeaParser.getGGAData();
      last_data.utc = convert_utc_time(gga.utcTime);
      last_data.lat = gga.latitude;
      last_data.lon = gga.longitude;
    }

    // Handle GSV data
    if (nmeaParser.isGSVParsed()) {
      GSVData gsv = nmeaParser.getGSVData();
      last_data.sats = gsv.satellitesInView;
    }
  }
  smart_delay();
  delay(2000);
}