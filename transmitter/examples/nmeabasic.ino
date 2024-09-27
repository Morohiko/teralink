#include <nmeaparser.h>

NMEAParser nmeaParser;

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
  Serial1.begin(115200, SERIAL_8N1, GNSS_RXD, GNSS_TXD); // Adjust baud rate if necessary
  
  // Wait for GNSS module to initialize
  delay(1000);
}

void loop() {
  while (Serial1.available()) {
    char c = Serial1.read();

    if (c == '\n' || c == '\r') {
      if (bufferIndex > 0) {
        inputBuffer[bufferIndex] = '\0';
        String sentence = String(inputBuffer);
        bufferIndex = 0;

        sentence.trim();

        Serial.print("Received: ");
        Serial.println(sentence);

        // Parse the sentence
        if (nmeaParser.parseSentence(sentence)) {
          // Handle GGA data
          if (nmeaParser.isGGAParsed()) {
            GGAData gga = nmeaParser.getGGAData();
            Serial.println("=== GGA Parsed Data ===");
            Serial.print("UTC Time: ");
            Serial.println(gga.utcTime);
            
            Serial.print("Latitude: ");
            Serial.println(gga.latitude, 6);
            
            Serial.print("Longitude: ");
            Serial.println(gga.longitude, 6);
                       
            Serial.print("Number of Satellites: ");
            Serial.println(gga.numSatellites);
            
            Serial.println("=========================");
          }
          
          
          // Handle GSV data
          if (nmeaParser.isGSVParsed()) {
            GSVData gsv = nmeaParser.getGSVData();
            Serial.println("=== GSV Parsed Data ===");
            Serial.print("Satellites in View: ");
            Serial.println(gsv.satellitesInView);
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
