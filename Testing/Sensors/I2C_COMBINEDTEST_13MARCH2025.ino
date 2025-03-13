#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>

/* Assign a unique ID to the ADXL345 */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);
Adafruit_ICM20948 icm;

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
}

void loop() {
  sensors_event_t accelEvent;
  sensors_event_t gyroEvent;
  sensors_event_t magEvent;
  sensors_event_t tempEvent;

  // Get ADXL345 readings
  accel.getEvent(&accelEvent);

  Serial.print("ADXL345 - X: ");
  Serial.print(accelEvent.acceleration.x);
  Serial.print(" Y: ");
  Serial.print(accelEvent.acceleration.y);
  Serial.print(" Z: ");
  Serial.println(accelEvent.acceleration.z);

  // Get ICM20948 readings
  icm.getEvent(&accelEvent, &gyroEvent, &tempEvent, &magEvent);

  Serial.print("ICM20948 - Accel X: ");
  Serial.print(accelEvent.acceleration.x);
  Serial.print(" Y: ");
  Serial.print(accelEvent.acceleration.y);
  Serial.print(" Z: ");
  Serial.println(accelEvent.acceleration.z);

  Serial.print("ICM20948 - Gyro X: ");
  Serial.print(gyroEvent.gyro.x);
  Serial.print(" Y: ");
  Serial.print(gyroEvent.gyro.y);
  Serial.print(" Z: ");
  Serial.println(gyroEvent.gyro.z);

  Serial.print("ICM20948 - Temp: ");
  Serial.print(tempEvent.temperature);
  Serial.println(" C");

  Serial.println();
  delay(500);
}