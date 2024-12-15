// Define the DHT11 pin
#define DHTPIN PD2  // DHT11 is connected to pin 2 on PORTD

// I2C Slave Address
#define I2C_ADDRESS 2

// Variables for temperature and humidity
volatile uint8_t temperature = 0;  // Store temperature as an integer
volatile uint8_t humidity = 0;     // Store humidity as an integer

// Function Prototypes
void initGPIO();                        // Initialize GPIO for DHT11
uint8_t readDHT11(uint8_t* temp, uint8_t* hum); // Read DHT11 sensor
void initI2C();                         // Initialize I2C in Slave Mode
void sendI2CData();                     // I2C data transmission handler

int main(void) {
  // Initialize GPIO for DHT11 and I2C for communication
  initGPIO();
  initI2C();

  // Main loop
  while (1) {
    // Read temperature and humidity from the DHT11 sensor
    if (readDHT11(&temperature, &humidity) == 0) {
      // Successfully read data, loop continues
    }

    // Small delay before the next read
    _delay_ms(2000);
  }

  return 0;
}

// Initialize GPIO for DHT11
void initGPIO() {
  // Set DHTPIN as input initially
  DDRD &= ~(1 << DHTPIN); // Clear DHTPIN bit to make it input
  PORTD |= (1 << DHTPIN); // Enable pull-up resistor
}

// Read DHT11 sensor data
uint8_t readDHT11(uint8_t* temp, uint8_t* hum) {
  uint8_t data[5] = {0}; // Buffer to store data from DHT11
  uint8_t i, j;

  // Start signal
  DDRD |= (1 << DHTPIN);  // Set DHTPIN as output
  PORTD &= ~(1 << DHTPIN); // Pull DHTPIN low
  _delay_ms(18);           // Hold for at least 18ms
  PORTD |= (1 << DHTPIN);  // Pull DHTPIN high
  _delay_us(20);           // Hold high for 20-40us
  DDRD &= ~(1 << DHTPIN);  // Set DHTPIN as input
  _delay_us(40);

  // Wait for response from DHT11
  if ((PIND & (1 << DHTPIN))) return 1; // Error: No response
  _delay_us(80);
  if (!(PIND & (1 << DHTPIN))) return 1; // Error: No response
  _delay_us(80);

  // Read 5 bytes of data (40 bits)
  for (j = 0; j < 5; j++) {
    for (i = 0; i < 8; i++) {
      // Wait for the start of the bit
      while (!(PIND & (1 << DHTPIN))); // Wait until pin goes high
      _delay_us(30);

      // Read the bit
      if (PIND & (1 << DHTPIN)) {
        data[j] |= (1 << (7 - i)); // If pin is still high after 30us, it's a 1
        while (PIND & (1 << DHTPIN)); // Wait for pin to go low
      }
    }
  }

  // Checksum validation
  if (data[4] != (data[0] + data[1] + data[2] + data[3])) return 2; // Error: Checksum mismatch

  // Store the results
  *hum = data[0];
  *temp = data[2];

  return 0; // Success
}

// Initialize I2C in Slave Mode
void initI2C() {
  // Set the slave address
  TWAR = (I2C_ADDRESS << 1); // TWI (Two-Wire Interface) Address Register

  // Enable TWI and ACK
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);

  // Wait for data requests
  while (!(TWCR & (1 << TWINT)));
}

// Handle I2C Data Transmission
void sendI2CData() {
  // Load temperature and humidity into TWI Data Register
  TWDR = temperature; // Send temperature
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Clear interrupt flag
  while (!(TWCR & (1 << TWINT))); // Wait for transmission to complete

  TWDR = humidity; // Send humidity
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Clear interrupt flag
  while (!(TWCR & (1 << TWINT))); // Wait for transmission to complete
}
