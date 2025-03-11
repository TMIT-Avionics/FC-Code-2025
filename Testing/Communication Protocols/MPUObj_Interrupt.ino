#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>

// Create MPU6050 object
Adafruit_MPU6050 mpu;

// Pin for MPU6050 interrupt
const int MPU6050_INT_PIN = 4;

// Flag for interrupt handling
volatile bool mpuInterrupt = false;

// ISR for MPU6050 interrupt
void IRAM_ATTR onMPUInterrupt() {
  mpuInterrupt = true;
}

void setup() {
  Serial.begin(115200);
  //delay(1000);
  // SDA 33 SCL 36
  // Initialize I2C communication
  Wire.begin();

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1);
  }

  // Set MPU6050 interrupt pin
  pinMode(MPU6050_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MPU6050_INT_PIN), onMPUInterrupt, RISING);

  Serial.println("MPU6050 initialized with interrupt.");
}

void loop() {
  if (mpuInterrupt) {
    mpuInterrupt = false; // Reset interrupt flag

    // Read accelerometer and gyroscope data
    sensors_event_t accel, gyro, temp;
    mpu.getEvent(&accel, &gyro, &temp);

    // Print accelerometer data
    Serial.print("Accel X: ");
    Serial.print(accel.acceleration.x);
    Serial.print(" m/s^2, Y: ");
    Serial.print(accel.acceleration.y);
    Serial.print(" m/s^2, Z: ");
    Serial.print(accel.acceleration.z);
    Serial.println(" m/s^2");

    // Print gyroscope data
    Serial.print("Gyro X: ");
    Serial.print(gyro.gyro.x);
    Serial.print(" rad/s, Y: ");
    Serial.print(gyro.gyro.y);
    Serial.print(" rad/s, Z: ");
    Serial.print(gyro.gyro.z);
    Serial.println(" rad/s");

    // Print temperature
    Serial.print("Temperature: ");
    Serial.print(temp.temperature);
    Serial.println(" *C");

    Serial.println();
  }
}
