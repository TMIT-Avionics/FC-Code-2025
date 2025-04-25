#include <SPI.h>
#include <MS5611_SPI.h>
#include <SD.h>

#define RYLR    Serial1

#define MS_CS   10
#define MS_SDA  11
#define MS_SDO  12
#define MS_SCL  13

#define T_0     308.15
#define L       0.0065
#define P_0     101325
#define R       8.31432
#define g       9.80665

const float ALT_MUL = (T_0/L);
const float ALT_EXP = (R*L/g);

#define LAUNCH_THRESHOLD    10
#define APOGEE_CONDITION    1.5
#define APOGEE_THRESHOLD    5
#define MAIN_ALTITUDE       457
#define RECOVERY_CONDITION  0.2
#define RECOVERY_THRESHOLD  10

MS5611_SPI MS5611(MS_CS, MS_SDA, MS_SDO, MS_SCL);
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
    Serial.println("Error in read");
    return lastRead;
  }
  return MS5611.getPressure();
}

float getAltitude (float pressure) {
    // return ALT_MUL * (1 - pow(pressure/P_0, ALT_EXP));
    return 44330.0*(1.0 - pow((float)(pressure*100)/P_0, 0.1902949));
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
  sendRYLR("CODE START-UP");
  unsigned long timeout = millis() + 2000;
  while (!RYLR.available() && millis() < timeout);
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
    flightLog = SD.open("flight.csv", FILE_WRITE);
    if (flightLog) {
      flightLog.println("Time(ms);Pressure(mbar);State");
      flightLog.flush();
    } else {
      Serial.println("Could not open log file");
    }
  } else {
    Serial.println("SD card could not be initialized");
  }

  if (checkRYLR()) {
    Serial.println("Telemetry established");
  } else {
    Serial.println("Telemetry could not be established");
  }

  float lastRead = readMSPressure(P_0 / 100.0);
  float pressure;

  while (1) {
    pressure = readMSPressure(lastRead);
    flightTransition(pressure, lastRead);

    String data = String(millis()) + ";" + String(pressure) + ";" + String(getAltitude(pressure)) + ";" + String(currentState);
    Serial.println(data);
    sendRYLR(data);

    if (flightLog) {
      flightLog.println(data);
      flightLog.flush();
    }

    lastRead = pressure;
    delay(200);
  }
}
