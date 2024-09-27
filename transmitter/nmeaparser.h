#ifndef NMEA_PARSER_H
#define NMEA_PARSER_H

#include <Arduino.h>

// Structure to hold GGA data
struct GGAData {
  String utcTime;
  float latitude;      // In decimal degrees
  String latDirection; // N or S
  float longitude;     // In decimal degrees
  String lonDirection; // E or W
  int fixQuality;
  int numSatellites;
  float hdop;
  float altitude;
  String altitudeUnit;
  float geoidHeight;
  String geoidHeightUnit;
  String diffRefStationId;
};

// Structure to hold GSA data
struct GSAData {
  String mode1;
  String mode2;
  int satellitesUsed;
  int satellitePRNs[12];
  float pdop;
  float hdop;
  float vdop;
};

// Structure to hold satellite information in GSV data
struct SatelliteInfo {
  String PRN;
  int elevation; // Degrees
  int azimuth;   // Degrees
  int SNR;       // dB
};

// Structure to hold GSV data
struct GSVData {
  int totalMessages;
  int messageNumber;
  int satellitesInView;
  SatelliteInfo satellites[4];
};

class NMEAParser {
  public:
    NMEAParser();
    
    // Parse a single NMEA sentence
    bool parseSentence(String sentence);
    
    // Getters for parsed data
    bool isGGAParsed();
    GGAData getGGAData();
    
    bool isGSAParsed();
    GSAData getGSAData();
    
    bool isGSVParsed();
    GSVData getGSVData();
    
  private:
    // Parsed data storage
    GGAData ggaData;
    bool ggaParsed;
    
    GSAData gsaData;
    bool gsaParsed;
    
    GSVData gsvData;
    bool gsvParsed;
    
    // Helper functions
    float convertToDecimalDegrees(String coord, String direction);
    bool verifyChecksum(String sentence);
    int splitString(String data, char delimiter, String *array, int maxElements);
    
    // Parsing functions
    bool parseGGA(String sentence);
    bool parseGSA(String sentence);
    bool parseGSV(String sentence);
};

#endif
