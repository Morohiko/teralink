#include "nmeaparser.h"

// Constructor
NMEAParser::NMEAParser() {
  // Initialize parsed flags
  ggaParsed = false;
  gsaParsed = false;
  gsvParsed = false;
}

// Public method to parse a sentence
bool NMEAParser::parseSentence(String sentence) {
  // Verify checksum
  if (!verifyChecksum(sentence)) {
    // Invalid checksum
    return false;
  }
  
  // Extract sentence type
  if (!sentence.startsWith("$")) {
    // Not a valid NMEA sentence
    return false;
  }
  
  String sentenceType = sentence.substring(3, 6); // Extract characters 4-6
  
  if (sentenceType == "GGA") {
    // Parse GGA
    return parseGGA(sentence);
  }
  else if (sentenceType == "GSA") {
    // Parse GSA
    return parseGSA(sentence);
  }
  else if (sentenceType == "GSV") {
    // Parse GSV
    return parseGSV(sentence);
  }
  
  // Unsupported sentence type
  return false;
}

// Getter for GGA data
bool NMEAParser::isGGAParsed() {
  return ggaParsed;
}

GGAData NMEAParser::getGGAData() {
  return ggaData;
}

// Getter for GSA data
bool NMEAParser::isGSAParsed() {
  return gsaParsed;
}

GSAData NMEAParser::getGSAData() {
  return gsaData;
}

// Getter for GSV data
bool NMEAParser::isGSVParsed() {
  return gsvParsed;
}

GSVData NMEAParser::getGSVData() {
  return gsvData;
}

// Helper function to convert NMEA coordinates to Decimal Degrees
float NMEAParser::convertToDecimalDegrees(String coord, String direction) {
  if (coord.length() < 4) return 0.0;
  
  // Split into degrees and minutes
  int dotIndex = coord.indexOf('.');
  if (dotIndex == -1 || dotIndex < 2) return 0.0;
  
  String degreesStr = coord.substring(0, dotIndex - 2);
  String minutesStr = coord.substring(dotIndex - 2);
  
  float degrees = degreesStr.toFloat();
  float minutes = minutesStr.toFloat();
  
  // Convert to decimal degrees
  float decimalDegrees = degrees + (minutes / 60.0);
  
  // Apply direction
  if (direction == "S" || direction == "W") {
    decimalDegrees *= -1;
  }
  
  return decimalDegrees;
}

// Helper function to verify checksum
bool NMEAParser::verifyChecksum(String sentence) {
  // Find the asterisk
  int asteriskIndex = sentence.indexOf('*');
  if (asteriskIndex == -1) return false;
  
  // Extract the checksum from the sentence
  String checksumStr = sentence.substring(asteriskIndex + 1);
  unsigned int receivedChecksum = strtoul(checksumStr.c_str(), NULL, 16);
  
  // Calculate checksum by XORing all characters between '$' and '*'
  unsigned int calculatedChecksum = 0;
  for (unsigned int i = 1; i < asteriskIndex; i++) { // Start after '$'
    calculatedChecksum ^= sentence[i];
  }
  
  // Compare checksums
  return (calculatedChecksum == receivedChecksum);
}

// Helper function to split string by delimiter
int NMEAParser::splitString(String data, char delimiter, String *array, int maxElements) {
  int count = 0;
  int start = 0;
  int end = data.indexOf(delimiter);
  
  while (end != -1 && count < maxElements) {
    array[count++] = data.substring(start, end);
    start = end + 1;
    end = data.indexOf(delimiter, start);
  }
  
  // Add the last field
  if (count < maxElements) {
    array[count++] = data.substring(start);
  }
  
  return count;
}

// Implementation for GGA
bool NMEAParser::parseGGA(String sentence) {
  // Remove the starting '$' and split by ','
  String data = sentence.substring(1, sentence.indexOf('*')); // Remove '$' and checksum
  String fields[15]; // GGA has 15 fields including checksum
  int fieldCount = splitString(data, ',', fields, 15);
  
  if (fieldCount < 14) {
    // Incomplete GGA sentence
    return false;
  }
  
  // Populate GGAData structure
  ggaData.utcTime = fields[1];
  
  if (fields[2] != "" && fields[3] != "" && fields[4] != "" && fields[5] != "") {
    ggaData.latitude = convertToDecimalDegrees(fields[2], fields[3]);
    ggaData.latDirection = fields[3];
    ggaData.longitude = convertToDecimalDegrees(fields[4], fields[5]);
    ggaData.lonDirection = fields[5];
  } else {
    ggaData.latitude = 0.0;
    ggaData.longitude = 0.0;
    ggaData.latDirection = "";
    ggaData.lonDirection = "";
  }
  
  ggaData.fixQuality = fields[6].toInt();
  ggaData.numSatellites = fields[7].toInt();
  ggaData.hdop = fields[8].toFloat();
  ggaData.altitude = fields[9].toFloat();
  ggaData.altitudeUnit = fields[10];
  ggaData.geoidHeight = fields[11].toFloat();
  ggaData.geoidHeightUnit = fields[12];
  ggaData.diffRefStationId = fields[13];
  
  ggaParsed = true;
  return true;
}

// Implementation for GSA
bool NMEAParser::parseGSA(String sentence) {
  // Remove the starting '$' and split by ','
  String data = sentence.substring(1, sentence.indexOf('*')); // Remove '$' and checksum
  String fields[19]; // GSA can have up to 19 fields including checksum
  int fieldCount = splitString(data, ',', fields, 19);
  
  if (fieldCount < 17) {
    // Incomplete GSA sentence
    return false;
  }
  
  // Populate GSAData structure
  gsaData.mode1 = fields[1];
  gsaData.mode2 = fields[2];
  gsaData.pdop = fields[15].toFloat();
  gsaData.hdop = fields[16].toFloat();
  gsaData.vdop = fields[17].toFloat();
  
  // Parse satellite PRNs (fields 3 to 14)
  gsaData.satellitesUsed = 0;
  for (int i = 0; i < 12; i++) {
    if (fields[3 + i] != "") {
      gsaData.satellitePRNs[i] = fields[3 + i].toInt();
      gsaData.satellitesUsed++;
    } else {
      gsaData.satellitePRNs[i] = 0;
    }
  }
  
  gsaParsed = true;
  return true;
}

// Implementation for GSV
bool NMEAParser::parseGSV(String sentence) {
  // Remove the starting '$' and split by ','
  String data = sentence.substring(1, sentence.indexOf('*')); // Remove '$' and checksum
  String fields[20]; // GSV can have up to 20 fields including checksum
  int fieldCount = splitString(data, ',', fields, 20);
  
  if (fieldCount < 4) {
    // Incomplete GSV sentence
    return false;
  }
  
  // Populate GSVData structure
  gsvData.totalMessages = fields[1].toInt();
  gsvData.messageNumber = fields[2].toInt();
  gsvData.satellitesInView = fields[3].toInt();
  
  // Each satellite has 4 fields: PRN, elevation, azimuth, SNR
  // Starting from field 4
  int satelliteIndex = 0;
  for (int i = 4; i < fieldCount - 1; i += 4) {
    if (i + 3 >= fieldCount) break; // Prevent out-of-bounds
    
    gsvData.satellites[satelliteIndex].PRN = fields[i];
    gsvData.satellites[satelliteIndex].elevation = fields[i + 1].toInt();
    gsvData.satellites[satelliteIndex].azimuth = fields[i + 2].toInt();
    gsvData.satellites[satelliteIndex].SNR = fields[i + 3].toInt();
    
    satelliteIndex++;
    if (satelliteIndex >= 4) break; // Limit to 4 satellites per GSV sentence
  }
  
  gsvParsed = true;
  return true;
}
