#include <nmeaparser.h>

NMEAParser nmeaParser;

#define GNSS_RXD 25
#define GNSS_TXD 26

#define BF_RXD 18
#define BF_TXD 19

#define BUFFER_SIZE 256

char inputBuffer[BUFFER_SIZE];
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for Serial to be ready
  }
  Serial.println("nmeaparser"); 
  Serial1.begin(115200, SERIAL_8N1, GNSS_RXD, GNSS_TXD);
  Serial2.begin(115200, SERIAL_8N1, BF_RXD, BF_TXD);
  
  delay(1000);
}

void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();
    Serial2.write(c);

    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        inputBuffer[bufferIndex] = '\0';
        String sentence = String(inputBuffer);
        bufferIndex = 0; // Reset buffer

        // Trim any whitespace
        sentence.trim();

        Serial.print("Received: ");
        Serial.println(sentence);

        if (nmeaParser.parseSentence(sentence)) {
          if (nmeaParser.isGGAParsed()) {
            GGAData gga = nmeaParser.getGGAData();
            Serial.println("=== GGA Parsed Data ===");
            Serial.print("UTC Time: ");
            Serial.println(gga.utcTime);
            
            Serial.print("Latitude: ");
            Serial.println(gga.latitude, 6);
            
            Serial.print("Longitude: ");
            Serial.println(gga.longitude, 6);
            
            Serial.print("Fix Quality: ");
            Serial.println(gga.fixQuality);
            
            Serial.print("Number of Satellites: ");
            Serial.println(gga.numSatellites);
            
            Serial.print("HDOP: ");
            Serial.println(gga.hdop);
            
            Serial.print("Altitude: ");
            Serial.print(gga.altitude);
            Serial.print(" ");
            Serial.println(gga.altitudeUnit);
            
            Serial.print("Geoid Height: ");
            Serial.print(gga.geoidHeight);
            Serial.print(" ");
            Serial.println(gga.geoidHeightUnit);
            Serial.println("=========================");
          }
          
          // Handle GSA data
          if (nmeaParser.isGSAParsed()) {
            GSAData gsa = nmeaParser.getGSAData();
            Serial.println("=== GSA Parsed Data ===");
            Serial.print("Mode 1: ");
            Serial.println(gsa.mode1);
            Serial.print("Mode 2: ");
            Serial.println(gsa.mode2);
            Serial.print("Satellites Used: ");
            Serial.println(gsa.satellitesUsed);
            Serial.print("PDOP: ");
            Serial.println(gsa.pdop);
            Serial.print("HDOP: ");
            Serial.println(gsa.hdop);
            Serial.print("VDOP: ");
            Serial.println(gsa.vdop);
            Serial.println("=========================");
          }
          
          // Handle GSV data
          if (nmeaParser.isGSVParsed()) {
            GSVData gsv = nmeaParser.getGSVData();
            Serial.println("=== GSV Parsed Data ===");
            Serial.print("Total Messages: ");
            Serial.println(gsv.totalMessages);
            Serial.print("Message Number: ");
            Serial.println(gsv.messageNumber);
            Serial.print("Satellites in View: ");
            Serial.println(gsv.satellitesInView);
            Serial.println("Satellite Information:");
            for (int i = 0; i < 4; i++) {
              if (gsv.satellites[i].PRN != "") {
                Serial.print("  Satellite ");
                Serial.print(i + 1);
                Serial.println(":");
                Serial.print("    PRN: ");
                Serial.println(gsv.satellites[i].PRN);
                Serial.print("    Elevation: ");
                Serial.println(gsv.satellites[i].elevation);
                Serial.print("    Azimuth: ");
                Serial.println(gsv.satellites[i].azimuth);
                Serial.print("    SNR: ");
                Serial.println(gsv.satellites[i].SNR);
              }
            }
            Serial.println("=========================");
          }
        }
        else {
          Serial.println("Failed to parse the sentence.");
        }
      }
    }
    else {
      // Add character to buffer if space permits
      if (bufferIndex < (BUFFER_SIZE - 1)) {
        inputBuffer[bufferIndex++] = c;
      }
      else {
        // Buffer overflow, reset
        Serial.println("Buffer Overflow. Resetting buffer.");
        bufferIndex = 0;
      }
    }
  }
  
  // Optional: Add a small delay to prevent overwhelming the loop
  delay(10);
}

