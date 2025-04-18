#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include "MS5611_SPI.h"
#include <SD.h>

#define MS_CS       10
#define MS_MOSI     11
#define MS_MISO     12
#define MS_SCK      13
#define FILE_NAME   "logsdata.txt"
#define DATAPOINTS  14 // 3 + 9 + 2

/* Assign a unique ID to the ADXL345 */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_ICM20948 icm;
MS5611_SPI MS5611(MS_CS, MS_MOSI, MS_MISO, MS_SCK);

float dataArray[DATAPOINTS] = {0};
int idx = 0;
File dataFile;
String packet;

void appendData(int value) {
    if (idx >= DATAPOINTS) idx = 0;
    dataArray[idx] = value;
    idx++;
}

void logData(String packet) {
  dataFile = SD.open(FILE_NAME, FILE_WRITE);
  if (dataFile) {
      dataFile.println(packet);
      dataFile.close();
      Serial.println("\t...Data written to SD Card successfully");
  } else {
      Serial.println("Error opening file");
  }
  delay(10);
}

void compileCollectedData() {
    packet = "";
    for (int i = 0; i < DATAPOINTS; i++) {
        packet += String(dataArray[i]);
        if (i < DATAPOINTS - 1) packet += ",";
    }

    logData(packet);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Initializing sensors...");

  // Start I2C communication
  Wire.begin();

  // Initialize ADXL345
  if (!accel.begin()) {
    Serial.println("Failed to find ADXL345");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);

  // Initialize ICM20948
  if (!icm.begin_I2C()) {
    Serial.println("Failed to find ICM20948");
    while (1);
  }

  Serial.println("Both sensors initialized!");

  if (MS5611.begin()) {
    Serial.println("\t...MS5611 Initialized!");
  } else {
    while(1);
  }

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("Card failed, or not present");
    while (1) {
      // No SD card, so don't do anything more - stay stuck here
    }
  }
  Serial.println("SD card initialized.");

  dataFile = SD.open(FILE_NAME, FILE_WRITE);
  if (dataFile)
    Serial.println("LogFile created successfully");
  else
    Serial.println("LogFile creation failed");
  dataFile.close();
  Serial.println("-------------# LOGGING  READY #-------------");
}

void loop() {
  // ADXL345 Data Acquisition
  sensors_event_t event_adxl; 
  accel.getEvent(&event_adxl);
  appendData(event_adxl.acceleration.x);
  appendData(event_adxl.acceleration.y);
  appendData(event_adxl.acceleration.z);

  // ICM20948 Data Acquisition
  sensors_event_t accel_icm;
  sensors_event_t gyro_icm;
  sensors_event_t temp_icm;
  sensors_event_t mag_icm;
  icm.getEvent(&accel_icm, &gyro_icm, &temp_icm, &mag_icm);
  appendData(accel_icm.acceleration.x);
  appendData(accel_icm.acceleration.y);
  appendData(accel_icm.acceleration.z);
  appendData(gyro_icm.gyro.x);
  appendData(gyro_icm.gyro.y);
  appendData(gyro_icm.gyro.z);
  appendData(mag_icm.magnetic.x);
  appendData(mag_icm.magnetic.y);
  appendData(mag_icm.magnetic.z);

  // MS5611 Data Acquisition
  int result = MS5611.read();
  if (result != MS5611_READ_OK) {
    Serial.print("Error in read: "); Serial.println(result);
  }
  appendData(MS5611.getTemperature());
  appendData(MS5611.getPressure());

  // Data Logging
  compileCollectedData();

  // Display Data in Serial
  Serial.println("#------------------------------------------#");

  Serial.println("-----# ADXL345:");
  Serial.print("\t\tX: "); Serial.print(event_adxl.acceleration.x); Serial.print("  ");
  Serial.print("\tY: "); Serial.print(event_adxl.acceleration.y); Serial.print("  ");
  Serial.print("\tZ: "); Serial.print(event_adxl.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");

  Serial.println("-----# ICM20948: ");
  Serial.print("\t\tTemperature ");
  Serial.print(temp_icm.temperature);
  Serial.println(" deg C");

  Serial.print("\t\tAccel X: ");
  Serial.print(accel_icm.acceleration.x);
  Serial.print(" \tY: ");
  Serial.print(accel_icm.acceleration.y);
  Serial.print(" \tZ: ");
  Serial.print(accel_icm.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.print("\t\tGyro X: ");
  Serial.print(gyro_icm.gyro.x);
  Serial.print(" \tY: ");
  Serial.print(gyro_icm.gyro.y);
  Serial.print(" \tZ: ");
  Serial.print(gyro_icm.gyro.z);
  Serial.println(" radians/s ");

  Serial.println("-----# MS5611: ");
  Serial.print("\t\tT: ");
  Serial.print(MS5611.getTemperature(), 2);
  Serial.print(" \tP: ");
  Serial.print(MS5611.getPressure(), 2);
  Serial.println();

  delay(200);
}