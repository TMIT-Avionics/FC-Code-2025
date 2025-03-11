#include <Wire.h>

// MPU6050 I2C address
#define MPU6050_ADDR 0x68

// MPU6050 registers
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_GYRO_XOUT_H 0x43
#define MPU6050_REG_TEMP_OUT_H 0x41
#define MPU6050_REG_PWR_MGMT_1 0x6B
#define MPU6050_REG_INT_ENABLE 0x38
#define MPU6050_REG_INT_STATUS 0x3A
#define MPU6050_REG_SMPLRT_DIV 0x19
#define MPU6050_REG_CONFIG 0x1A

// Interrupt pin
const int MPU6050_INT_PIN = 4;

// Flag for interrupt handling
volatile bool mpuInterrupt = false;

// ISR for MPU6050 interrupt
void IRAM_ATTR onMPUInterrupt() {
  mpuInterrupt = true;
}

// Function to initialize the MPU6050
void initializeMPU6050() {
  // Wake up MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_REG_PWR_MGMT_1);
  Wire.write(0x00); // Clear sleep mode
  Wire.endTransmission();

  // Set sample rate to 1 kHz
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_REG_SMPLRT_DIV);
  Wire.write(0x07); // Sample rate = 1 kHz / (1 + 7) = 125 Hz
  Wire.endTransmission();

  // Set DLPF to 42 Hz
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_REG_CONFIG);
  Wire.write(0x03); // DLPF_CFG = 3
  Wire.endTransmission();

  // Enable data ready interrupt
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_REG_INT_ENABLE);
  Wire.write(0x01); // Enable Data Ready interrupt
  Wire.endTransmission();
}

// Function to read raw data from registers
void readMPU6050Raw(int16_t &ax, int16_t &ay, int16_t &az, int16_t &gx, int16_t &gy, int16_t &gz, int16_t &temp) {
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(MPU6050_REG_ACCEL_XOUT_H);
  Wire.endTransmission();

  Wire.requestFrom(MPU6050_ADDR, 14); // Read 14 bytes: Accel (6), Temp (2), Gyro (6)
  if (Wire.available() == 14) {
    ax = (Wire.read() << 8) | Wire.read();
    ay = (Wire.read() << 8) | Wire.read();
    az = (Wire.read() << 8) | Wire.read();
    temp = (Wire.read() << 8) | Wire.read();
    gx = (Wire.read() << 8) | Wire.read();
    gy = (Wire.read() << 8) | Wire.read();
    gz = (Wire.read() << 8) | Wire.read();
  }
}

// Function to scale raw data to human-readable units
void convertToScaledValues(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz, int16_t temp,
                           float &axScaled, float &ayScaled, float &azScaled, float &gxScaled, float &gyScaled, float &gzScaled, float &tempScaled) {
  axScaled = ax / 16384.0; // Accelerometer sensitivity: 16384 LSB/g
  ayScaled = ay / 16384.0;
  azScaled = az / 16384.0;
  gxScaled = gx / 131.0; // Gyroscope sensitivity: 131 LSB/(°/s)
  gyScaled = gy / 131.0;
  gzScaled = gz / 131.0;
  tempScaled = (temp / 340.0) + 36.53; // Temperature formula from datasheet
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);

  // Initialize MPU6050
  initializeMPU6050();

  // Configure interrupt pin
  pinMode(MPU6050_INT_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(MPU6050_INT_PIN), onMPUInterrupt, RISING);

  Serial.println("MPU6050 initialized with interrupt.");
}

void loop() {
  if (mpuInterrupt) {
    mpuInterrupt = false; // Reset interrupt flag

    // Read raw data
    int16_t accelX, accelY, accelZ, gyroX, gyroY, gyroZ, tempRaw;
    readMPU6050Raw(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, tempRaw);

    // Convert raw data to scaled values
    float accelXScaled, accelYScaled, accelZScaled, gyroXScaled, gyroYScaled, gyroZScaled, tempScaled;
    convertToScaledValues(accelX, accelY, accelZ, gyroX, gyroY, gyroZ, tempRaw,
                          accelXScaled, accelYScaled, accelZScaled, gyroXScaled, gyroYScaled, gyroZScaled, tempScaled);

    // Print scaled data
    Serial.print("Accel X: ");
    Serial.print(accelXScaled, 2);
    Serial.print(" m/s^2, Y: ");
    Serial.print(accelYScaled, 2);
    Serial.print(" m/s^2, Z: ");
    Serial.print(accelZScaled, 2);
    Serial.println(" m/s^2");

    Serial.print("Gyro X: ");
    Serial.print(gyroXScaled, 2);
    Serial.print(" rad/s, Y: ");
    Serial.print(gyroYScaled, 2);
    Serial.print(" rad/s, Z: ");
    Serial.print(gyroZScaled, 2);
    Serial.println(" rad/s");

    Serial.print("Temperature: ");
    Serial.print(tempScaled, 2);
    Serial.println(" *C");

    Serial.println();
  }
}
