/* Program for Integrated Test on LTS_IRIS board. To test functionality of:
 *  - ADXL345
 *  - ICM20948
 *  - MS5611
 *  - RYLR998
 *  - SD Card Adapter
 *  - D4184-based Pyro channel
 * to implement: interrupts where possible, 
@date 15-03-2025 | @author thrustMIT 2024-25 */

#include <Wire.h>                         // Communication Protocol Library for I2C
#include <Adafruit_Sensor.h>              // Adafruit Common Sensor Library
#include <Adafruit_ADXL345_U.h>           // ADXL345 Library
#include <Adafruit_ICM20X.h>              // ICM20948 Library
#include <Adafruit_ICM20948.h>
#include <MS5611_SPI.h>                   // MS5611 SPI Library

/* IMPORTANT PINOUTS
  - ADXL345 (I2C1)
      ADXL_INT      --> IO2
      SDO           --> GND
  - MS5611
      Chip Select   --> IO27
  - ICM20948
      ICM_INT       --> RXD0
*/

#define MS5611_CHIPSELECT     27
#define PYRO_A                26        // MOSFET
#define PYRO_B                25        // D4184
#define RXD2                  16        // RYLR998 RX
#define TXD2                  17        // RYLR998 TX

/* OBJECT CREATION FOR SENSORS
  - MS5611
  - ADXL345
  - ICM20948
*/

MS5611_SPI MS5611(MS5611_CHIPSELECT, 13, 12, 14);   // ESP32 SW SPI
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_ICM20948 icm;

/* VARIABLES
  - RYLR string variables
  - real-time data variables
*/

String data, response, transmit, input;

/* SETUP PROGRAM
  - Initialize and confirm Serial
  - Initialize all sensors
*/

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("-----# LTS-Iris Batch Test 15-03-2025 #-----");
  Serial.println("");
  Serial.println("----------# Sensor Initialization #---------");

  // ADXL345 Initialization
  if(!accel.begin()) {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    while(1);
  } else {
    Serial.println("\t...ADXL345 Initialized!");
  }

  // ICM20948 Initialization
  if (!icm.begin_I2C()) {
    /* There was a problem detecting the ICM20948 ... check your connections */
    Serial.println("Failed to find ICM20948 chip");
    while (1);
  } else {
    Serial.println("\t...ICM20948 Initialized!");
  }

  // MS5611 Initialization
  SPI.begin();
  if (MS5611.begin() == true) {
    Serial.println("MS5611 found.");
    while(1);
  } else {
    Serial.println("\t...MS5611 Initialized!");
  }

  Serial.println("All Sensors Initialized!");
}

/* LOOP FUNCTION
*/

void loop() {
  // ADXL345 Data Acquisition
  sensors_event_t event_adxl; 
  accel.getEvent(&event_adxl);

  // ICM20948 Data Acquisition
  sensors_event_t accel_icm;
  sensors_event_t gyro_icm;
  sensors_event_t temp_icm;
  sensors_event_t mag_icm;
  icm.getEvent(&accel_icm, &gyro_icm, &temp_icm, &mag_icm);

  // MS5611 Data Acquisition
  int result = MS5611.read();
  if (result != MS5611_READ_OK) {
    Serial.print("Error in read: "); Serial.println(result);
  }

  // Display Data in Serial
  Serial.println("-----# ADXL345:")
  Serial.print("\t\tX: "); Serial.print(event.acceleration.x); Serial.print("  ");
  Serial.print("\tY: "); Serial.print(event.acceleration.y); Serial.print("  ");
  Serial.print("\tZ: "); Serial.print(event.acceleration.z); Serial.print("  ");Serial.println("m/s^2 ");

  Serial.println("-----# ICM20948: ")
  Serial.print("\t\tTemperature ");
  Serial.print(temp.temperature);
  Serial.println(" deg C");

  Serial.print("\t\tAccel X: ");
  Serial.print(accel.acceleration.x);
  Serial.print(" \tY: ");
  Serial.print(accel.acceleration.y);
  Serial.print(" \tZ: ");
  Serial.print(accel.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.print("\t\tGyro X: ");
  Serial.print(gyro.gyro.x);
  Serial.print(" \tY: ");
  Serial.print(gyro.gyro.y);
  Serial.print(" \tZ: ");
  Serial.print(gyro.gyro.z);
  Serial.println(" radians/s ");
  Serial.println();

  Serial.println("-----# MS5611: ")
  Serial.print("\t\tT: ");
  Serial.print(MS5611.getTemperature(), 2);
  Serial.print(" \tP: ");
  Serial.print(MS5611.getPressure(), 2);
  Serial.print(" \tt: ");
  Serial.println();

  delay(2000);
}


