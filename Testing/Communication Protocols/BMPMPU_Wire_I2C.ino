#include <Wire.h>

// BMP280 and MPU6050 I2C addresses
#define BMP280_ADDR 0x76 // or 0x77
#define MPU6050_ADDR 0x68

// BMP280 registers
#define BMP280_REG_TEMP_MSB 0xFA
#define BMP280_REG_PRESS_MSB 0xF7
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG 0xF5

// MPU6050 registers
#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_PWR_MGMT_1 0x6B

// Initialize BMP280
void initializeBMP280() {
    Wire.beginTransmission(BMP280_ADDR);
    Wire.write(BMP280_REG_CTRL_MEAS); // Control register
    Wire.write(0x27); // Normal mode, temp and press enabled, oversampling 1
    Wire.endTransmission();

    Wire.beginTransmission(BMP280_ADDR);
    Wire.write(BMP280_REG_CONFIG); // Config register
    Wire.write(0xA0); // Standby 1000ms, filter coefficient 2
    Wire.endTransmission();
}

// Initialize MPU6050
void initializeMPU6050() {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_REG_PWR_MGMT_1); // Power management register
    Wire.write(0x00); // Wake up the MPU6050
    Wire.endTransmission();
}

// Function to read temperature from BMP280
int32_t readBMP280Temperature() {
    Wire.beginTransmission(BMP280_ADDR);
    Wire.write(BMP280_REG_TEMP_MSB); // Temperature MSB register
    Wire.endTransmission();
    Wire.requestFrom(BMP280_ADDR, 3); // Read 3 bytes of temperature data

    if (Wire.available() >= 3) {
        int32_t temp_raw = (Wire.read() << 12) | (Wire.read() << 4) | (Wire.read() >> 4);
        // Temperature compensation calculation here, simplified as an example
        return temp_raw; // Placeholder
    }
    return 0;
}

// Function to read pressure from BMP280
int32_t readBMP280Pressure() {
    Wire.beginTransmission(BMP280_ADDR);
    Wire.write(BMP280_REG_PRESS_MSB); // Pressure MSB register
    Wire.endTransmission();
    Wire.requestFrom(BMP280_ADDR, 3); // Read 3 bytes of pressure data

    if (Wire.available() >= 3) {
        int32_t press_raw = (Wire.read() << 12) | (Wire.read() << 4) | (Wire.read() >> 4);
        // Pressure compensation calculation here, simplified as an example
        return press_raw; // Placeholder
    }
    return 0;
}

// Function to read acceleration data from MPU6050
void readMPU6050Accel(int16_t &x, int16_t &y, int16_t &z) {
    Wire.beginTransmission(MPU6050_ADDR);
    Wire.write(MPU6050_REG_ACCEL_XOUT_H); // Starting register for accel data
    Wire.endTransmission();
    Wire.requestFrom(MPU6050_ADDR, 6); // Read 6 bytes of acceleration data

    if (Wire.available() == 6) {
        x = (Wire.read() << 8) | Wire.read();
        y = (Wire.read() << 8) | Wire.read();
        z = (Wire.read() << 8) | Wire.read();
    }
}

void setup() {
    Wire.begin();           // Initialize I2C
    Serial.begin(115200);   // Start Serial communication
    while(!Serial);
    delay(5000);

    initializeBMP280();
    initializeMPU6050();
}

void loop() {
    // Read temperature and pressure from BMP280
    int32_t temperature = readBMP280Temperature();
    int32_t pressure = readBMP280Pressure();
    Serial.print("Temperature (BMP280): ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Pressure (BMP280): ");
    Serial.print(pressure / 100.0);
    Serial.println(" hPa");

    // Read acceleration data from MPU6050
    int16_t accelX, accelY, accelZ;
    readMPU6050Accel(accelX, accelY, accelZ);
    Serial.print("Accel X: ");
    Serial.print(accelX);
    Serial.print(" | Accel Y: ");
    Serial.print(accelY);
    Serial.print(" | Accel Z: ");
    Serial.println(accelZ);

    delay(1000); // Delay for readability
}