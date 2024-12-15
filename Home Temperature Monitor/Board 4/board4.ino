// Define EEPROM addresses
#define TEMP_ADDRESS 0 // EEPROM address for temperature
#define HUM_ADDRESS 50 // EEPROM address for humidity

// I2C Slave Address
#define I2C_ADDRESS 3

// Function prototypes
void initI2C();              // Initialize I2C in Slave Mode
void receiveI2CData();       // Handle I2C data reception
void writeEEPROM(uint16_t address, uint8_t data); // Write data to EEPROM
uint8_t readEEPROM(uint16_t address);            // Read data from EEPROM

volatile uint8_t receivedData[8]; // Buffer for received I²C data

int main(void) {
  // Initialize I2C
  initI2C();

  // Main loop
  while (1) {
    // Continuously waiting for data through I²C (handled by interrupts)
  }

  return 0;
}

// Initialize I2C in Slave Mode
void initI2C() {
  // Set the slave address for Slave 4
  TWAR = (I2C_ADDRESS << 1); // TWI Address Register (shifted left by 1)

  // Enable TWI, ACK, and Interrupts
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWIE) | (1 << TWINT);

  // Enable global interrupts
  sei();
}

// Handle I2C data reception
ISR(TWI_vect) {
  static uint8_t byteCount = 0;

  // Check TWI status code for received data
  if ((TWSR & 0xF8) == 0x60 || (TWSR & 0xF8) == 0x68) {
    // Own SLA+W received, ACK returned
    byteCount = 0;
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Prepare for next byte
  } else if ((TWSR & 0xF8) == 0x80) {
    // Data byte has been received
    if (byteCount < sizeof(receivedData)) {
      receivedData[byteCount++] = TWDR; // Read received byte
    }
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Prepare for next byte
  } else if ((TWSR & 0xF8) == 0xA0) {
    // Stop or repeated start condition received
    if (byteCount == 8) {
      // Process received data
      uint8_t temperature = receivedData[0]; // First byte is temperature
      uint8_t humidity = receivedData[4];    // Fifth byte is humidity

      // Store data in EEPROM
      writeEEPROM(TEMP_ADDRESS, temperature);
      writeEEPROM(HUM_ADDRESS, humidity);
    }
    byteCount = 0; // Reset byte count
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Acknowledge reception
  } else {
    // Handle other cases
    TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Clear TWI interrupt
  }
}

// Write a byte to EEPROM
void writeEEPROM(uint16_t address, uint8_t data) {
  // Wait for completion of previous write
  while (EECR & (1 << EEPE));

  EEAR = address; // Set EEPROM address
  EEDR = data;    // Set EEPROM data register
  EECR = (1 << EEMPE); // Enable Master Write
  EECR |= (1 << EEPE); // Start EEPROM write
}

// Read a byte from EEPROM
uint8_t readEEPROM(uint16_t address) {
  // Wait for completion of previous write
  while (EECR & (1 << EEPE));

  EEAR = address; // Set EEPROM address
  EECR |= (1 << EERE); // Start EEPROM read
  return EEDR;         // Return data from EEPROM Data Register
}
