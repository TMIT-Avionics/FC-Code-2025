#include <SPI.h>
#include <MS5611_SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <SD.h>
#include <SparkFun_u-blox_GNSS_v3.h>

#define RYLR        Serial2
#define WireGNSS    Wire // Connect using the Wire port. Change this if required
#define gnssAddress 0x42 // The default I2C address for u-blox modules is 0x42. Change this if required
#define FILE_NAME   "tst17may.csv"

#define MS_CS   10
#define MS_SDA  11
#define MS_SDO  12
#define MS_SCL  13

#define T_0     308.15 //adjust this to temperature on that day
#define L       0.0065
#define P_0     101325 //adjust this to the specific sea level altitude on that day
#define R       287 //8.31432 this must be 287, not 8.314
#define g       9.80665

typedef enum {
  LATITUDE,
  LONGITUDE,
  ALTITUDE
} Location;

const float ALT_MUL = (T_0/L);
const float ALT_EXP = (R*L/g);

#define LAUNCH_THRESHOLD              3
#define MACH_INHIBIT_DELAY            18000
#define COASTING_THRESHOLD            3
#define APOGEE_PRESSURE_CONDITION     1.5
#define APOGEE_ACCL_CONDITION         2
#define APOGEE_THRESHOLD              5
#define MAIN_ALTITUDE                 457
#define RECOVERY_CONDITION            0.2
#define RECOVERY_THRESHOLD            10

MS5611_SPI MS5611(MS_CS, MS_SDA, MS_SDO, MS_SCL);
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
SFE_UBLOX_GNSS myGNSS;
File flightLog;
int count = 0;
long launchTime = 0;

typedef enum {
  STANDBY,
  LAUNCH,
  COASTING,
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

bool initADXL() {
  if (!accel.begin()) {
    return false;
  }
  accel.setRange(ADXL345_RANGE_16_G);
  return true;
}

void getADXLAccl (float accl[3]) {
  sensors_event_t event; 
  accel.getEvent(&event);

  accl[0] = event.acceleration.x;
  accl[1] = event.acceleration.y;
  accl[2] = event.acceleration.z;
}

void flightTransition(float pressure, float lastRead, float accl[3]) {
  float lateralAccl = sqrt(accl[0]*accl[0] + accl[2]*accl[2]);
  float verticalAccl = -accl[2];
  float totalAccl = sqrt(accl[0]*accl[0] + accl[1]*accl[1] + accl[2]*accl[2]);
  float acclAngle = atan(lateralAccl/verticalAccl);

  if (totalAccl > 40*g) {
    switch(currentState) {
      case STANDBY:
        if (pressure < lastRead) count++;
        else count = 0;

        if (count >= LAUNCH_THRESHOLD) {
          currentState = LAUNCH;
          launchTime = millis();
          count = 0;
        }
        break;

      case LAUNCH:
      case COASTING:
        if (pressure - APOGEE_PRESSURE_CONDITION > lastRead) count++;
        else count = 0;

        if (count >= APOGEE_THRESHOLD && (millis() - launchTime > MACH_INHIBIT_DELAY)) {
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
  } else {
    switch(currentState) {
      case STANDBY:
        if (totalAccl > 5*g || pressure < lastRead) count++;
        else count = 0;

        if (count >= LAUNCH_THRESHOLD) {
          currentState = LAUNCH;
          count = 0;
        }
        break;

      case LAUNCH:
        if (acclAngle < 0) count++;
        else count = 0;

        if (count >= COASTING_THRESHOLD) {
          currentState = COASTING;
          count = 0;
        }
        break;

      case COASTING:
        if (pressure - APOGEE_PRESSURE_CONDITION > lastRead || fabs(verticalAccl) < APOGEE_ACCL_CONDITION) count++;
        else count = 0;

        if (count >= APOGEE_THRESHOLD && (millis() - launchTime > MACH_INHIBIT_DELAY)) {
          currentState = APOGEE;
          launchTime = millis();
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
}

bool initGNSS() {
  WireGNSS.begin(); // Start I2C
  // myGNSS.begin(WireGNSS, gnssAddress);

  if (!myGNSS.begin(WireGNSS, gnssAddress)) 
    return false;
  myGNSS.setI2COutput(COM_TYPE_UBX);
  return true;
}

void getLocation (int32_t location[3]) {
  if (myGNSS.getPVT(1) == true)
  {
    location[0] = myGNSS.getLatitude();
    location[1] = myGNSS.getLongitude();
    location[2] = myGNSS.getAltitudeMSL();
  } 
  // else {
  //   Serial.println("No new data...");
  // }
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
  Serial.println("Am in RYLR init");
  sendRYLR("CODE START-UP");
  unsigned long timeout = millis() + 2000;
  while (!RYLR.available() && millis() < timeout){
    Serial.println("Am waiting here");
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

  if (initADXL()) {
    Serial.println("ADXL initialized");
  } else {
    Serial.println("ADXL could not be initialized");
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

  Serial.println("Am exiting SD init");

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

  float lastRead = readMSPressure(P_0 / 100.0);
  float pressure;
  int32_t loc[3];
  float accl[3];

  while (1) {
    pressure = readMSPressure(lastRead);
    getADXLAccl(accl);
    flightTransition(pressure, lastRead, accl);
    getLocation(loc);

    String data = String(millis()) + ";" +
              String(pressure, 2) + ";" +
              String(getAltitude(pressure), 2) + ";" +
              String(accl[0], 2) + ";" +
              String(accl[1], 2) + ";" +
              String(accl[2], 2) + ";" +
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
