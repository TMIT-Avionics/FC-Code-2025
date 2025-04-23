#include <Wire.h> //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3

SFE_UBLOX_GNSS myGNSS;

#define WireGNSS Wire // Connect using the Wire1 port. Change this if required
#define gnssAddress 0x42 // The default I2C address for u-blox modules is 0x42. Change this if required

String data, response, transmit, input;

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

void setup()
{
  // pinMode(17, OUTPUT_OPENDRAIN);
  // pinMode(16, OUTPUT_OPENDRAIN);
  Serial.begin(115200);
  Serial2.begin(57600);
  delay(1000); 
  // Serial.println("SparkFun u-blox Example");

  WireGNSS.begin(); // Start I2C
  // Wire.begin();
  // WireGNSS.setSDA(17);
  // WireGNSS.setSCL(16);
  

  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  while (myGNSS.begin(WireGNSS, 0x42) == false) //Connect to the u-blox module using our custom port and address
  {
    Serial.println(F("u-blox GNSS not detected. Retrying..."));
    delay (1000);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  
  //myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Optional: save (only) the communications port settings to flash and BBR

  Serial.println("\nGNSS & RYLR998 Test\n");
}

void loop()
{
  // Request (poll) the position, velocity and time (PVT) information.
  // The module only responds when a new position is available. Default is once per second.
  // getPVT() returns true when new data is received.
  if (myGNSS.getPVT() == true)
  {
    int32_t latitude = myGNSS.getLatitude();
    Serial.print(F("Lat: "));
    Serial.print(latitude);

    int32_t longitude = myGNSS.getLongitude();
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    int32_t altitude = myGNSS.getAltitudeMSL(); // Altitude above Mean Sea Level
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));

    Serial.println();

    input = String(latitude) + ";" + String(longitude) + ";" + String(altitude);
    sendData(input);

  } else {
    Serial.println("No new data...");
    delay(1000);
  }
}