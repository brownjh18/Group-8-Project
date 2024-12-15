#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2
#define DHTTYPE DHT11
DHT_Unified dht(DHTPIN, DHTTYPE);

float temperature = 0;
float humidity = 0;

void setup() {
  Serial.begin(9600);  // Debugging
  Wire.begin(8);       // Set I2C address of this board as 8
  dht.begin();
  Serial.println("Board 1: Sensor setup complete");
  Wire.onRequest(sendData);
}

void loop() {
  sensors_event_t tempEvent, humEvent;
  dht.temperature().getEvent(&tempEvent);
  dht.humidity().getEvent(&humEvent);

  if (!isnan(tempEvent.temperature) && !isnan(humEvent.relative_humidity)) {
    temperature = tempEvent.temperature;
    humidity = humEvent.relative_humidity;

    // Debugging: Log data to Serial Monitor
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" C, Hum: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Sensor error: Unable to read data");
  }

  delay(2000); // Wait 2 seconds between readings
}

void sendData() {
  // Pack temperature and humidity into a byte array
  uint8_t data[8];
  memcpy(data, &temperature, 4); // Copy 4 bytes of temperature
  memcpy(data + 4, &humidity, 4); // Copy 4 bytes of humidity

  // Send the packed data
  Wire.write(data, 8);
}
