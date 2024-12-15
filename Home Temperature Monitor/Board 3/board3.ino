#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 2       // DHT11 connected to pin 2
#define DHTTYPE DHT11  // Define sensor type
DHT_Unified dht(DHTPIN, DHTTYPE);

float temperature = 0, humidity = 0;

void setup() {
  Wire.begin(2); // Set I²C address as 2
  dht.begin();
  Wire.onRequest(sendData); // Handle data requests
  Serial.begin(9600);
  Serial.println("Slave2 (DHT11) Initialized");
}

void loop() {
  sensors_event_t tempEvent, humEvent;
  dht.temperature().getEvent(&tempEvent);
  dht.humidity().getEvent(&humEvent);

  if (!isnan(tempEvent.temperature) && !isnan(humEvent.relative_humidity)) {
    temperature = tempEvent.temperature;
    humidity = humEvent.relative_humidity;
    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" C, Hum: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Error reading DHT sensor!");
  }

  delay(2000); // Update every 2 seconds
}

void sendData() {
  Wire.write((byte *)&temperature, 4);
  Wire.write((byte *)&humidity, 4);
}
