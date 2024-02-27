#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <WiFi.h>
#include <SPI.h>
#include <SD.h>
#include "time.h"

Adafruit_BME680 bme680;  // Create a BME680 sensor object
Adafruit_BME680 bme688;  // Create a BME688 sensor object

// WiFi Configuration (Replace "YourWiFiSSID" and "YourWiFiPassword" with your actual WiFi credentials)
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";

// SD Card Pin Configuration
#define SD_MOSI 13
#define SD_MISO 12
#define SD_SCLK 14
#define SD_CS   15

File dataFile;
bool fileAppended = false;

void setup() {
  Serial.begin(115200);

  // Initialize the BME680 and BME688 sensors
  if (!bme680.begin(0x77) || !bme688.begin(0x76)) { // Replace with the actual sensor addresses if necessary
    Serial.println("Could not find a valid BME680 or BME688 sensor, check wiring!");
    while (1);
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // SD card initialization
  SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card Mount Failed");
    return;
  }

  // Append CSV header with parameter names
  appendHeader("/BME680_BME688_data.csv"); // Change the filename if needed
  // Initialize the RTC (Real-Time Clock)
  configTime(0, 0, "pool.ntp.org");  // Use NTP for time synchronization
}

void loop() {
  if (!bme680.performReading() || !bme688.performReading()) {
    Serial.println("Failed to perform BME680 or BME688 reading!");
    return;
  }
 
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Error synchronizing time");
    return;
  }

  char timestamp[30];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

  // Read sensor data
  float temperature0 = bme680.temperature;
  float pressure0 = bme680.pressure / 100.0F;
  float humidity0 = bme680.humidity;
  float gasResistance0 = bme680.gas_resistance / 1000.0F;;
  uint8_t gasIndex0 = bme680.readGas();

  float temperature8 = bme688.temperature;
  float pressure8 = bme688.pressure / 100.0F;
  float humidity8 = bme688.humidity;
  float gasResistance8 = bme688.gas_resistance / 1000.0F;;
  uint8_t gasIndex8 = bme688.readGas();

  // Print sensor data to serial monitor
  Serial.print("Timestamp: ");
  Serial.println(timestamp);

  Serial.print("Temperature_680 = ");
  Serial.print(temperature0);
  Serial.println(" *C");

  // Print other sensor data...

  // Write sensor data to SD card
  String dataLine680_688 = String(timestamp) + "," + String(temperature0) + "," + String(temperature8) + "," +
                           String(pressure0) + "," + String(pressure8) + "," + String(humidity0) + "," + String(humidity8) +
                           "," + String(gasResistance0) + "," + String(gasResistance8) + "," + String(gasIndex0) + "," + String(gasIndex8) + "\n";

  appendFile("/BME680_BME688_data.csv", dataLine680_688); // Change the filename if needed

  delay(5000); // Adjust delay as needed
}

void appendHeader(const char *path) {
  File file = SD.open(path);
  if (!file) {
    file = SD.open(path, FILE_WRITE);
    if (file) {
      String header = "Timestamp,Temperature_680,Temperature_688,Pressure_680,Pressure_688,Humidity_680,Humidity_688,Gas_Resistance_680,Gas_Resistance_688,Gas_Index_680,Gas_Index_688\n";
      file.print(header);
      file.close();
      fileAppended = true;
    } else {
      Serial.println("Failed to open file for writing header");
    }
  } else {
    fileAppended = true;
  }
}

void appendFile(const char *path, const String &data) {
  File file = SD.open(path, FILE_APPEND);

  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }

  if (file.print(data)) {
    Serial.println("Data appended");
  } else {
    Serial.println("Append failed");
  }

  file.close();
}
