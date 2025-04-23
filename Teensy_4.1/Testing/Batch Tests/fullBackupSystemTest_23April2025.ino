#include <Wire.h> //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3
#include <MS5611_SPI.h>
#include <SD.h>

SFE_UBLOX_GNSS myGNSS;
MS5611_SPI MS5611(10, 11, 12, 13); 

#define WireGNSS Wire // Connect using the Wire1 port. Change this if required
#define gnssAddress 0x42 // The default I2C address for u-blox modules is 0x42. Change this if required
#define FILE_NAME "fulltest.csv"

String data, response, transmit, input;
File logfile;
float temperature, pressure;
int32_t longitude, latitude, altitude;

String parseRYLR(String input) {
  int start = input.indexOf(',') + 1;
  start = input.indexOf(',', start) + 1;
  int end = input.indexOf(',', start);
  String parsed = input.substring(start, end);
  parsed.trim();
  return parsed;  
}

void receiveData() {
  if (Serial2.available()) {
    response = Serial2.readStringUntil('\n');
    response = parseRYLR(response);
    Serial.println("DATA RECEIVED: " + response);
    //Serial2.clear();
  }
}

void sendData(String data) {
  transmit = "AT+SEND=0," + String(data.length()) + "," + data + "\r\n";
  Serial2.print(transmit);
  delay(10);
  Serial.print("TRANSMITTED: " + transmit);
}

void readMS5611() {
  int result = MS5611.read();
  if (result != MS5611_READ_OK) {
    Serial.print("Error in read: ");
    Serial.println(result);
  } else {
    temperature = MS5611.getTemperature();
    pressure = MS5611.getPressure(); // Keep as in your code

    unsigned long timestamp = millis();

    Serial.print("T:\t");
    Serial.print(temperature, 2);
    Serial.print("\tP:\t");
    Serial.print(pressure, 2);
    Serial.println();

    // Open, write, and close the file each time
    logfile = SD.open(FILE_NAME, FILE_WRITE);
    if (logfile) {
      logfile.print(timestamp);
      logfile.print(",");
      logfile.print(temperature, 2);
      logfile.print(",");
      logfile.print(pressure, 2);
      logfile.print(",");
      logfile.print(longitude);
      logfile.print(",");
      logfile.print(latitude);
      logfile.print(",");
      logfile.print(altitude);
      logfile.println();
      logfile.close();
    } else {
      Serial.println("Could not open log file!");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(57600);
  delay(1000); 

  WireGNSS.begin(); // Start I2C

  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  while (myGNSS.begin(WireGNSS, 0x42) == false) //Connect to the u-blox module using our custom port and address
  {
    Serial.println(F("u-blox GNSS not detected. Retrying..."));
    delay (1000);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  
  //myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Optional: save (only) the communications port settings to flash and BBR

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card initialization failed!");
    while (1) {
      delay(100);
    }
  }
  Serial.println("SD card initialized.");

  SPI.begin();

  if (MS5611.begin() == true) {
    Serial.println("MS5611 found.");
  } else {
    Serial.println("MS5611 not found. halt.");
    while (1) {
      delay(100);
    }
  }
  // Serial.println();

  // Optional: Write CSV header if file does not exist
  if (!SD.exists(FILE_NAME)) {
    File logfile = SD.open(FILE_NAME, FILE_WRITE);
    if (logfile) {
      logfile.println("millis,temp_C,pressure_mbar,long,lat,alt");
      logfile.close();
    }
  }

  MS5611.setOversampling(OSR_STANDARD);
  Serial.println("\nFull Batch Test\n");
}

void loop()
{
  // Request (poll) the position, velocity and time (PVT) information.
  // The module only responds when a new position is available. Default is once per second.
  // getPVT() returns true when new data is received.
  if (myGNSS.getPVT() == true)
  {
    latitude = myGNSS.getLatitude();
    Serial.print(F("Lat: "));
    Serial.print(latitude);

    longitude = myGNSS.getLongitude();
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    altitude = myGNSS.getAltitudeMSL(); // Altitude above Mean Sea Level
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));

    Serial.println();
    readMS5611();

    input = String(pressure) + ";" + String(temperature) + ";" + String(latitude) + ";" + String(longitude) + ";" + String(altitude);
    sendData(input);

  } else {
    Serial.println("No new data...");
    delay(1000);
  }
}