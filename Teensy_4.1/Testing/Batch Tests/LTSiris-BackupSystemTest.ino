#include <SPI.h>
#include <MS5611_SPI.h>
#include <SD.h>
#include <SparkFun_u-blox_GNSS_v3.h>

#define RYLR        Serial2
#define WireGNSS    Wire // Connect using the Wire1 port. Change this if required
#define gnssAddress 0x42 // The default I2C address for u-blox modules is 0x42. Change this if required
#define FILE_NAME   "tst27apr.csv"

#define MS_CS   10
#define MS_SDA  11
#define MS_SDO  12
#define MS_SCL  13

#define T_0     308.15
#define L       0.0065
#define P_0     101325
#define R       8.31432
#define g       9.80665

typedef enum {
  LATITUDE,
  LONGITUDE,
  ALTITUDE
} Location;

const float ALT_MUL = (T_0/L);
const float ALT_EXP = (R*L/g);

#define LAUNCH_THRESHOLD    10
#define APOGEE_CONDITION    1.5
#define APOGEE_THRESHOLD    5
#define MAIN_ALTITUDE       457
#define RECOVERY_CONDITION  0.2
#define RECOVERY_THRESHOLD  10

MS5611_SPI MS5611(MS_CS, MS_SDA, MS_SDO, MS_SCL);
SFE_UBLOX_GNSS myGNSS;
File flightLog;
int count = 0;

typedef enum {
  STANDBY,
  LAUNCH,
  APOGEE,
  MAIN,
  RECOVERY
} STATE;
STATE currentState = STANDBY;

float readMSPressure(float lastRead) {
  if (MS5611.read() != MS5611_READ_OK) {
    // Serial.println("Error in read");
    return lastRead;
  }
  return MS5611.getPressure();
}

float getAltitude (float pressure) {
    return ALT_MUL * (1.0 - pow((pressure*100)/P_0, ALT_EXP));
    // return 44330.0*(1.0 - pow((float)(pressure*100)/P_0, 0.1902949));
}

void flightTransition(float pressure, float lastRead) {
  switch(currentState) {
    case STANDBY:
      if (pressure < lastRead) count++;
      else count = 0;

      if (count >= LAUNCH_THRESHOLD) {
        currentState = LAUNCH;
        count = 0;
      }
      break;

    case LAUNCH:
      if (pressure - APOGEE_CONDITION > lastRead) count++;
      else count = 0;

      if (count >= APOGEE_THRESHOLD) {
        currentState = APOGEE;
        count = 0;
      }
      break;

    case APOGEE:
      if (getAltitude(pressure) < MAIN_ALTITUDE) currentState = MAIN;
      break;

    case MAIN:
      if (fabs(pressure - lastRead) < RECOVERY_CONDITION) count++;
      else count = 0;

      if (count >= RECOVERY_THRESHOLD) {
        currentState = RECOVERY;
        count = 0;
      }
      break;

    case RECOVERY:
      break;
  }
}

bool initGNSS() {
  WireGNSS.begin(); // Start I2C
  myGNSS.begin(WireGNSS, 0x42);

  if (myGNSS.begin(WireGNSS, 0x42)) return true;
  else return false;
}

void getLocation (int32_t location[3]) {
  if (myGNSS.getPVT(50) == true)
  {
    location[0] = myGNSS.getLatitude();
    location[1] = myGNSS.getLongitude();
    location[2] = myGNSS.getAltitudeMSL();
  } else {
    Serial.println("No new data...");
  }
}

String parseRYLR(String input) {
  int start = input.indexOf(',') + 1;
  start = input.indexOf(',', start) + 1;
  int end = input.indexOf(',', start);
  String parsed = input.substring(start, end);
  parsed.trim();
  return parsed;  
}

void sendRYLR(String transmit) {
  String packet = "AT+SEND=0," + String(transmit.length()) + "," + transmit + "\r\n";
  RYLR.print(packet);
  delay(10);
}

String receiveRYLR() {
  if (RYLR.available()) {
    String receive = RYLR.readStringUntil('\n');
    receive = parseRYLR(receive.trim());
    return receive;
  }
  return "NO_DATA";
}

bool checkRYLR() {
  // Serial.println("Am in RYLR init");
  sendRYLR("CODE START-UP");
  unsigned long timeout = millis() + 2000;
  while (!RYLR.available() && millis() < timeout){
    // Serial.println("Am in this here");
    delay(5);
  }
  return parseRYLR(receiveRYLR()) == "+OK";
}

int main () {
  Serial.begin(115200);
  RYLR.begin(57600);
  SPI.begin();

  if (MS5611.begin()) {
    Serial.println("MS5611 initialized");
  } else {
    Serial.println("MS5611 could not be initialized");
  }

  if (SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card initialized");
    flightLog = SD.open(FILE_NAME, FILE_WRITE);
    if (flightLog) {
      flightLog.println("Time(ms);Pressure(mbar);State");
      flightLog.flush();
    } else {
      Serial.println("Could not open log file");
    }
  } else {
    Serial.println("SD card could not be initialized");
  }

  // Serial.println("Am exiting SD init");

  if (checkRYLR()) {
    Serial.println("Telemetry established");
  } else {
    Serial.println("Telemetry could not be established");
  }

  if (initGNSS()) {
    Serial.println(F("u-blox GNSS detected."));
  } else {
    Serial.println(F("u-blox GNSS not detected."));
  }
  myGNSS.setI2COutput(COM_TYPE_UBX);

  float lastRead = readMSPressure(P_0 / 100.0);
  float pressure;
  int32_t loc[3];

  while (1) {
    pressure = readMSPressure(lastRead);
    flightTransition(pressure, lastRead);
    getLocation(loc);

    String data = String(millis()) + ";" +
              String(pressure, 2) + ";" +
              String(getAltitude(pressure), 2) + ";" +
              String(loc[LATITUDE]) + ";" +
              String(loc[LONGITUDE]) + ";" +
              String(loc[ALTITUDE]) + ";" +
              String(currentState);

    Serial.println(data);
   
    if (flightLog) {
      flightLog.println(data);
      flightLog.flush();
    }
    sendRYLR(data);

    lastRead = pressure;
    delay(2);
  }
}
