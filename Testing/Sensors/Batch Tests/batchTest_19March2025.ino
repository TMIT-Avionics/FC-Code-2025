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
#include <SPI.h>
#include <SD.h>

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

#define DATAPOINTS            14 //mpus => 3+9=12, MS=2, total=14

// HSPI pins for ESP32
#define HSPI_MISO   19
#define HSPI_MOSI   23
#define HSPI_SCLK   18
#define HSPI_CS     5
#define FILE_NAME   "/againtry.csv"

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

String data, response, transmit, packet;
float dataArray[DATAPOINTS] = {0};
int ind = 0;

SPIClass * hspi = NULL;
File dataFile;
int d = 0;

/* FUNCTIONS DECLARATIONS
  - appendData()
  - sendCollectedData()
*/

void appendData(float value) {
    if (ind >= DATAPOINTS) ind = 0;
    dataArray[ind] = value;
    ind++;
}

void logData(String packet) {
  dataFile = SD.open(FILE_NAME, FILE_APPEND);
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
    Serial.println("Transmitting collected data...");
    packet = "";
    for (int i = 0; i < DATAPOINTS; i++) {
        packet += String(dataArray[i]);
        if (i < DATAPOINTS - 1) packet += ",";
    }

    logData(packet);

    transmit = "AT+SEND=0," + String(packet.length()) + "," + packet + "\r\n";
    Serial1.print(transmit);
    Serial.println("\t...Transmitted String: " + transmit);
    Serial.println("\t...Data Packet Transmitted!");
    // Serial.println("Data transmission complete!");
}

/* SETUP PROGRAM
  - Initialize and confirm Serial
  - Initialize all sensors
*/

void setup() {
  Serial.begin(115200);
  Serial1.begin(57600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("-------------# LOGGING STATUS #-------------");
  // Initialize HSPI
  hspi = new SPIClass(HSPI);
  hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);

  if (!SD.begin(HSPI_CS, *hspi)) {
    Serial.println("\t...SD Card initialization failed!");
    return;
  } else {
    Serial.println("\t...SD Card Initialized Successfully!");
  }

  dataFile = SD.open(FILE_NAME, FILE_APPEND);
  if (dataFile)
    Serial.println("\t...LogFile created successfully");
  else
    Serial.println("\t...LogFile creation failed");
  Serial.println("-------------# LOGGING  READY #-------------");

  delay(5000);

  //
  packet = "AT+SEND=0,5,HELLO\r\n";
  Serial1.print(packet);

  Serial.println("-----# LTS-Iris Batch Test 15-03-2025 #-----");
  Serial.println("");
  Serial.println("----------# Sensor Initialization #---------");

  // ADXL345 Initialization
  if(!accel.begin()) {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    // while(1);
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
  // SPI.begin();
  if (MS5611.begin() == true) {
    Serial.println("\t...MS5611 Initialized!");
  } else {
    while(1);
  }

  Serial.println("\t...All Sensors Initialized!");
  Serial.println("------------# Data From Sensors #-----------");  

}

/* LOOP FUNCTION
*/

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

  // Data Transmission
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

  delay(2000);
}
