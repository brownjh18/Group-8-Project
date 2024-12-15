#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// DHT sensor setup
#define DHTPIN 2         // DHT11 data pin connected to Board 1 pin 2
#define DHTTYPE DHT11    // DHT 11 sensor type
DHT_Unified dht(DHTPIN, DHTTYPE);

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the I2C address (0x27 is common)

// Variables for temperature and humidity
float temperature = 0;
float humidity = 0;

// Define the number of readings to average
#define NUM_READINGS 10

// EEPROM addresses to store readings
int tempAddress = 0;  // Address for temperature
int humAddress = 50;  // Address for humidity (separated to avoid overwriting)

float storedTemperature[NUM_READINGS];
float storedHumidity[NUM_READINGS];
int readCount = 0;

// Variable to track if it's the first loop (for displaying average only once on startup)
bool firstLoop = true;

// Button setup
#define BUTTON_PIN 7  // Button pin connected to pin 7
bool buttonState = false;
bool lastButtonState = false;
bool systemOn = true;
unsigned long lastDebounceTime = 0;  // The last time the button state changed
unsigned long debounceDelay = 50;    // Debounce delay time in milliseconds

void setup() {
  // Initialize Serial Monitor for debugging
  Serial.begin(9600);

  // Initialize I2C communication and peripherals
  Wire.begin(8); // Set I2C address of this board as 8
  dht.begin();
  lcd.begin(16, 2);
  lcd.backlight();

  // Initialize stored readings from EEPROM if available
  for (int i = 0; i < NUM_READINGS; i++) {
    storedTemperature[i] = EEPROM.read(tempAddress + i);
    storedHumidity[i] = EEPROM.read(humAddress + i);
  }

  // Display average on first power-on
  if (firstLoop) {
    float avgTemp = 0;
    float avgHum = 0;

    for (int i = 0; i < NUM_READINGS; i++) {
      avgTemp += storedTemperature[i];
      avgHum += storedHumidity[i];
    }

    avgTemp /= NUM_READINGS;
    avgHum /= NUM_READINGS;

    // Display average temperature on LCD
    lcd.setCursor(0, 0);
    lcd.print("Avg Temp: ");
    lcd.print(avgTemp);
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Avg Hum: ");
    lcd.print(avgHum);
    lcd.print(" %");

    delay(5000); // Show the average for 5 seconds before switching to real-time display
    firstLoop = false; // Set firstLoop to false so this doesn't run again
  }

  // Set up I2C request handler
  Wire.onRequest(sendData);

  // Set up button pin
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  // Check for button press and debounce it
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis(); // Reset debounce timer
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == HIGH) {
        // Toggle system on/off
        systemOn = !systemOn;
        if (systemOn) {
          Serial.println("System ON");
          lcd.clear();
          lcd.print("System ON");
        } else {
          Serial.println("System OFF");
          lcd.clear();
          lcd.print("System OFF");
        }
      }
    }
  }

  lastButtonState = reading;

  if (systemOn) {
    // Read data from the DHT11 sensor
    sensors_event_t tempEvent, humEvent;
    dht.temperature().getEvent(&tempEvent);
    dht.humidity().getEvent(&humEvent);

    if (!isnan(tempEvent.temperature) && !isnan(humEvent.relative_humidity)) {
      // Update temperature and humidity variables
      temperature = tempEvent.temperature;
      humidity = humEvent.relative_humidity;

      // Debugging: Log data to Serial Monitor
      Serial.print("Temp: ");
      Serial.print(temperature);
      Serial.print(" C, Hum: ");
      Serial.print(humidity);
      Serial.println(" %");

      // Store the new readings in the arrays
      storedTemperature[readCount] = temperature;
      storedHumidity[readCount] = humidity;

      // Write the latest readings to EEPROM (keep the last NUM_READINGS)
      EEPROM.write(tempAddress + readCount, temperature);
      EEPROM.write(humAddress + readCount, humidity);

      // Increment the read count and wrap around if necessary
      readCount = (readCount + 1) % NUM_READINGS;

      // Display real-time temperature and humidity on LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp: ");
      lcd.print(temperature);
      lcd.print(" C");

      lcd.setCursor(0, 1);
      lcd.print("Hum: ");
      lcd.print(humidity);
      lcd.print(" %");
    } else {
      // Handle sensor read error
      Serial.println("Sensor error: Unable to read data");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sensor Error!");
    }

    delay(2000); // Wait 2 seconds between readings
  } else {
    // If the system is OFF, just wait
    delay(200);
  }
}

void sendData() {
  // Pack temperature and humidity into a byte array
  uint8_t data[8];
  memcpy(data, &temperature, 4); // Copy 4 bytes of temperature
  memcpy(data + 4, &humidity, 4); // Copy 4 bytes of humidity

  // Send the packed data
  Wire.write(data, 8);
}
