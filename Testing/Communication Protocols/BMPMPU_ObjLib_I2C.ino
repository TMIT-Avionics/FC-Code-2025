#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_MPU6050.h>

// Create objects for the sensors
Adafruit_BMP280 bmp;      // I2C interface for BMP280
Adafruit_MPU6050 mpu;     // I2C interface for MPU6050

#define SDA 32
#define SCL 33

void setup() {
  Serial.begin(115200);
  delay(5000);
  Wire.begin(SDA, SCL);

  // Initialize BMP280 sensor
  if (!bmp.begin(0x76)) {  // Check for BMP280 at address 0x76 (default)
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1) delay(10);
  }

  // Initialize MPU6050 sensor
  if (!mpu.begin()) {  // Check for MPU6050 at default address 0x68
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1) delay(10);
  }

  // BMP280 settings
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Normal mode
                  Adafruit_BMP280::SAMPLING_X2,     // Temp. oversampling x2
                  Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling x16
                  Adafruit_BMP280::FILTER_X16,      // Filtering x16
                  Adafruit_BMP280::STANDBY_MS_500); // Standby time 500 ms

  Serial.println("BMP280 and MPU6050 initialized!");
}

void loop() {
  // Read BMP280 data
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure() / 100.0F);  // Pressure in hPa
  Serial.println(" hPa");

  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude(1013.25));  // Assuming sea-level pressure of 1013.25 hPa
  Serial.println(" m");

  // Read MPU6050 data
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  Serial.print("Accel X: ");
  Serial.print(accel.acceleration.x);
  Serial.print(" m/s^2, Y: ");
  Serial.print(accel.acceleration.y);
  Serial.print(" m/s^2, Z: ");
  Serial.print(accel.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Gyro X: ");
  Serial.print(gyro.gyro.x);
  Serial.print(" rad/s, Y: ");
  Serial.print(gyro.gyro.y);
  Serial.print(" rad/s, Z: ");
  Serial.print(gyro.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" *C");

  Serial.println();
  delay(1000);  // Delay between readings
}
