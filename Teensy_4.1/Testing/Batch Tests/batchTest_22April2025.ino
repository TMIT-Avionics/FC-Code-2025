//    FILE: MS5611_test.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo application with SD logging for Teensy 4.1
//     URL: https://github.com/RobTillaart/MS5611_SPI

#include "MS5611_SPI.h"
#include <SD.h>

#define FILENAME "loggingMS.csv"

// MS5611_SPI(select, dataOut, dataIn, clock);
// --------------------------------------------
MS5611_SPI MS5611(10, 11, 12, 13);   // UNO SW SPI (works for Teensy too)
File logfile;

uint32_t start, stop;

void setup()
{
  // Serial.begin(115200);
  // while(!Serial);
  // Serial.println();
  // Serial.println(__FILE__);
  // Serial.print("MS5611_SPI_LIB_VERSION: ");
  // Serial.println(MS5611_SPI_LIB_VERSION);

  // Initialize SD card
  if (!SD.begin(BUILTIN_SDCARD)) {
    // Serial.println("SD card initialization failed!");
    while (1) {
      delay(100);
    }
  }
  // Serial.println("SD card initialized.");

  SPI.begin();

  if (MS5611.begin() == true) {
    // Serial.println("MS5611 found.");
  } else {
    // Serial.println("MS5611 not found. halt.");
    while (1) {
      delay(100);
    }
  }
  // Serial.println();

  // Optional: Write CSV header if file does not exist
  if (!SD.exists(FILENAME)) {
    File logfile = SD.open(FILENAME, FILE_WRITE);
    if (logfile) {
      logfile.println("millis,temp_C,pressure_mbar");
      logfile.close();
    }
  }
}

/*
  There are 5 oversampling settings, each corresponding to a different amount of milliseconds
  The higher the oversampling, the more accurate the reading will be, however the longer it will take.
  OSR_ULTRA_HIGH -> 8.22 millis
  OSR_HIGH       -> 4.11 millis
  OSR_STANDARD   -> 2.1 millis
  OSR_LOW        -> 1.1 millis
  OSR_ULTRA_LOW  -> 0.5 millis   Default = backwards compatible
*/
void loop() {
  MS5611.setOversampling(OSR_ULTRA_LOW);
  test();
  delay(1000);

  MS5611.setOversampling(OSR_LOW);
  test();
  delay(1000);

  MS5611.setOversampling(OSR_STANDARD);
  test();
  delay(1000);

  MS5611.setOversampling(OSR_HIGH);
  test();
  delay(1000);

  MS5611.setOversampling(OSR_ULTRA_HIGH);
  test();
  delay(1000);
  // Serial.println();
}

void test() {
  start = micros();
  int result = MS5611.read();
  stop = micros();
  if (result != MS5611_READ_OK) {
    // Serial.print("Error in read: ");
    // Serial.println(result);
  } else {
    float temperature = MS5611.getTemperature();
    float pressure = MS5611.getPressure(); // Keep as in your code

    unsigned long timestamp = millis();

    // Serial.print("T:\t");
    // Serial.print(temperature, 2);
    // Serial.print("\tP:\t");
    // Serial.print(pressure, 2);
    // Serial.print("\tt:\t");
    // Serial.print(stop - start);
    // Serial.println();

    // Open, write, and close the file each time
    logfile = SD.open(FILENAME, FILE_WRITE);
    if (logfile) {
      logfile.print(timestamp);
      logfile.print(",");
      logfile.print(temperature, 2);
      logfile.print(",");
      logfile.print(pressure, 2);
      logfile.println();
      logfile.close();
    } else {
      // Serial.println("Could not open log file!");
    }
  }
}

//  -- END OF FILE --
