#include <SPI.h>
#include <SD.h>

// HSPI pins for ESP32
#define HSPI_MISO   12
#define HSPI_MOSI   13
#define HSPI_SCLK   14
#define HSPI_CS     15

SPIClass * hspi = NULL;
File dataFile;
int d = 0;

void setup() {
    Serial.begin(115200);
    delay(5000);
    
    // Initialize HSPI
    hspi = new SPIClass(HSPI);
    hspi->begin(HSPI_SCLK, HSPI_MISO, HSPI_MOSI, HSPI_CS);
    
    // Configure SD card
    if (!SD.begin(HSPI_CS, *hspi)) {
        Serial.println("SD Card initialization failed!");
        return;
    }
    Serial.println("SD Card initialized successfully");
    File dataFile = SD.open("/plsswork.txt", FILE_WRITE);
    if (dataFile)
      Serial.println("LogFile created successfully");
    else
      Serial.println("LogFile creation failed");
    
    // Optional: You can also specify the clock speed
    // SD.begin(HSPI_CS, *hspi, 4000000);  // 4MHz clock speed
}

void loop() {
    dataFile = SD.open("/plsswork.txt", FILE_APPEND);
    if (dataFile) {
        dataFile.println(d);
        dataFile.close();
        Serial.println("Data written successfully");
        d++;
    } else {
        Serial.println("Error opening file");
    }
    
    delay(1000);
}