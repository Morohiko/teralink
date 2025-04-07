#include <nmeaparser.h>
#include <SPI.h>
#include <LoRa.h>

// Config
// #define WITH_BUZZER
// #define WITH_SERIAL_LOGS

#define MINIMUM_SATS_TO_SEND_LAST_GOOD 24
#define DELAY_TO_SEND 2000u

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

#ifdef WITH_BUZZER
  // buzzer pins
  #define BUZZER_PLUS 5
  #define BUZZER_GND 6
  #define BUZZER_IS_CONNECTED 3
#endif

#ifdef WITH_SERIAL_LOGS
  #define println(str) Serial.println("tx: " + str)
#else
  #define println(str)
#endif

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

struct gnss_data last_good_data = {
  .utc = String("00:00:00"),
  .lon = 0.0,
  .lat = 0.0,
  .sats = 0
};

unsigned long previousMillis = 0u;

void setup() {
#ifdef WITH_SERIAL_LOGS
  Serial.begin(115200);
#endif
  Serial1.begin(115200, SERIAL_8N1, GNSS_RXD, GNSS_TXD);
  
  SPI.begin(SCK, MISO, MOSI, SS);

  // Initialize LoRa
  LoRa.setPins(SS, RST, DIO0);

  while (!LoRa.begin(433E6)) {
    println("Starting LoRa failed!");
    delay(1000u);
  }

  println("LoRa Initializing OK!");

  previousMillis = millis();

#ifdef WITH_BUZZER
  pinMode(BUZZER_PLUS, OUTPUT);
  pinMode(BUZZER_GND, OUTPUT);
  pinMode(BUZZER_IS_CONNECTED, INPUT);
#endif

  // sleep
  delay(1000);
}

String generate_msg_to_send(struct gnss_data* data) {
  String to_send = "";
  if (data->utc.length() > 0) {
    to_send += "utc: " + data->utc + ", ";
  }
  to_send += "sats: " + String(data->sats) + ", ";
  to_send += "lat/lon: " + String(data->lat, 10) + "," + String(data->lon, 10);
  return to_send;
}

void send_lora_message() {
  String to_send = "";
  if (last_data.sats < MINIMUM_SATS_TO_SEND_LAST_GOOD &&
      last_good_data.sats > MINIMUM_SATS_TO_SEND_LAST_GOOD) {
    println("sats less then minimum, send saved good data")
    to_send += generate_msg_to_send(&last_good_data);
  }
  to_send += "\n";
  to_send += generate_msg_to_send(&last_data);

  LoRa.idle();

  LoRa.beginPacket();
  LoRa.print(to_send);
  LoRa.endPacket();

  LoRa.sleep();

  println(to_send);
}

void save_nmea_message(String sentence) {
  sentence.trim();

  // Parse the sentence
  if (!nmeaParser.parseSentence(sentence)) {
    println("can`t parce sentence");
    return;
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

  // Save for future if satellits enough
  if (last_data.sats >= MINIMUM_SATS_TO_SEND_LAST_GOOD) {
    last_good_data.sats = last_data.sats;
    last_good_data.utc = last_data.utc;
    last_good_data.lat = last_data.lat;
    last_good_data.lon = last_data.lon;
  }
}

void smart_delay() {
  unsigned long currentMillis = millis();
  unsigned long time_to_sleep = currentMillis - previousMillis;
  if (time_to_sleep >= DELAY_TO_SEND) {
    send_lora_message();
    previousMillis = currentMillis;
  }
}

inline String convert_utc_time(String ggaSentence) {
  return ggaSentence.substring(0, 2) + ":" + ggaSentence.substring(2, 4) + ":" + ggaSentence.substring(4, 6);
}

void loop() {
  while (Serial1.available()) {
#ifdef WITH_BUZZER
    int is_buzzer_connected = digitalRead(BUZZER_IS_CONNECTED);
    // turn on buzzer
    digitalWrite(BUZZER_PLUS, !is_buzzer_connected);
    digitalWrite(BUZZER_GND, 0);
#endif

    char c = Serial1.read();
    if (c != '\n' && c != '\r') {
      if (bufferIndex < (BUFFER_SIZE - 1)) {
        inputBuffer[bufferIndex++] = c;
      }
      else {
        // Buffer overflow, reset
        println("Buffer Overflow. Resetting buffer.");
        bufferIndex = 0;
      }
      continue;
    }

    if (bufferIndex <= 0)
      continue;

    inputBuffer[bufferIndex] = '\0';
    String sentence = String(inputBuffer);
    bufferIndex = 0;
    save_nmea_message(sentence);
  }
  smart_delay();
}