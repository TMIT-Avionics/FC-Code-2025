#include <Wire.h> //Needed for I2C to GNSS
#include <SparkFun_u-blox_GNSS_v3.h> //http://librarymanager/All#SparkFun_u-blox_GNSS_v3

SFE_UBLOX_GNSS myGNSS;

// #define WireGNSS Wire1 // Connect using the Wire1 port. Change this if required
#define gnssAddress 0x42 // The default I2C address for u-blox modules is 0x42. Change this if required

void setup()
{
  Serial.begin(115200);
  delay(1000); 
  Serial.println("SparkFun u-blox Example");

  // WireGNSS.begin(); // Start I2C
  Wire.begin();

  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  while (myGNSS.begin(/*WireGNSS*/Wire, gnssAddress) == false) //Connect to the u-blox module using our custom port and address
  {
    Serial.println(F("u-blox GNSS not detected. Retrying..."));
    delay (1000);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  
  //myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Optional: save (only) the communications port settings to flash and BBR
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
  } else {
    Serial.println("Fix not found yet...");
    delay(1000);
  }
}