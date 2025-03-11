#include <Adafruit_BMP280.h>  // Include Adafruit BMP280 library
#include <SPI.h>              // Include SPI library

// Define BMP280 object for SPI communication
#define BMP280_CS 15           // Chip select pin for BMP280 (you can choose another GPIO if needed)
#define BMP280_ID 0x58
#define BMP280_SCK 14          // Clock pin
#define BMP280_MISO 12         // MISO pin
#define BMP280_MOSI 13         // MOSI pin
Adafruit_BMP280 bmp280(BMP280_CS, BMP280_MOSI, BMP280_MISO, BMP280_SCK);  // Create BMP280 object
// Adafruit_BMP280(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);
// ESP32 WROOM-32 SPI pins

void setup() {
  // Start serial communication
  Serial.begin(115200);
  while (!Serial); // Wait until Serial is ready (for ESP32)
  delay(2000);

  // Initialize the BMP280 sensor with SPI
  if (!bmp280.begin()) {
    Serial.println("Failed to initialize BMP280! Check wiring.");
    while (1);
  }
  else {
    Serial.println("BMP280 set up successfully!");
  }

  // Optionally configure BMP280 settings (you can adjust sampling and filtering here)
  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                     Adafruit_BMP280::SAMPLING_X2,     // Temp oversampling
                     Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                     Adafruit_BMP280::FILTER_X16,      // Filtering
                     Adafruit_BMP280::STANDBY_MS_500); // Standby time
}

void loop() {
  // Read temperature and pressure from BMP280
  float temperature = bmp280.readTemperature();
  float pressure = bmp280.readPressure() / 100.0F; // Convert Pa to hPa (or mbar)

  // Print values to the serial monitor
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println(" hPa");

  delay(20);  // Wait 2 seconds before the next reading
}