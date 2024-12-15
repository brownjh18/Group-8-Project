// Define LCD control pins
#define RS PB0  // Register Select pin on PORTB
#define EN PB1  // Enable pin on PORTB
#define D4 PB2  // Data pin 4 on PORTB
#define D5 PB3  // Data pin 5 on PORTB
#define D6 PB4  // Data pin 6 on PORTB
#define D7 PB5  // Data pin 7 on PORTB

// I2C Slave Address
#define I2C_ADDRESS 1

// Function Prototypes
void initLCD();                    // Initialize the LCD
void lcdCommand(uint8_t cmd);      // Send command to LCD
void lcdData(uint8_t data);        // Send data to LCD
void lcdPrint(const char* str);    // Print string to LCD
void lcdClear();                   // Clear LCD display
void initI2C();                    // Initialize I2C in Slave Mode
void receiveI2CData();             // Handle I2C data reception

volatile uint8_t temperature = 0; // Store temperature data
volatile uint8_t humidity = 0;    // Store humidity data

int main(void) {
  // Initialize LCD and I2C
  initLCD();
  initI2C();

  // Display initialization message
  lcdClear();
  lcdPrint("Slave1 Initialized");

  // Main loop
  while (1) {
    // Update LCD display with the latest temperature and humidity
    lcdClear();
    lcdPrint("Temp: ");
    lcdData((temperature / 10) + '0'); // Display tens digit
    lcdData((temperature % 10) + '0'); // Display units digit
    lcdPrint(" C");
    lcdPrint(" Hum: ");
    lcdData((humidity / 10) + '0'); // Display tens digit
    lcdData((humidity % 10) + '0'); // Display units digit
    lcdPrint(" %");

    // Small delay before the next update
    _delay_ms(1000);
  }

  return 0;
}

// Initialize the LCD
void initLCD() {
  // Set LCD pins as output
  DDRB |= (1 << RS) | (1 << EN) | (1 << D4) | (1 << D5) | (1 << D6) | (1 << D7);

  // LCD initialization sequence
  _delay_ms(20);              // Wait for LCD to power up
  lcdCommand(0x03);           // Set 8-bit mode
  _delay_ms(5);
  lcdCommand(0x03);           // Repeat
  _delay_us(100);
  lcdCommand(0x03);           // Repeat
  lcdCommand(0x02);           // Set 4-bit mode
  lcdCommand(0x28);           // 2-line, 5x7 matrix
  lcdCommand(0x0C);           // Display ON, Cursor OFF
  lcdCommand(0x06);           // Entry mode, auto-increment
  lcdCommand(0x01);           // Clear display
}

// Send command to LCD
void lcdCommand(uint8_t cmd) {
  PORTB &= ~(1 << RS);         // RS = 0 for command
  PORTB = (PORTB & 0x0F) | (cmd & 0xF0); // Send higher nibble
  PORTB |= (1 << EN);
  _delay_us(1);
  PORTB &= ~(1 << EN);
  _delay_us(200);

  PORTB = (PORTB & 0x0F) | ((cmd << 4) & 0xF0); // Send lower nibble
  PORTB |= (1 << EN);
  _delay_us(1);
  PORTB &= ~(1 << EN);
  _delay_ms(2);
}

// Send data to LCD
void lcdData(uint8_t data) {
  PORTB |= (1 << RS);          // RS = 1 for data
  PORTB = (PORTB & 0x0F) | (data & 0xF0); // Send higher nibble
  PORTB |= (1 << EN);
  _delay_us(1);
  PORTB &= ~(1 << EN);
  _delay_us(200);

  PORTB = (PORTB & 0x0F) | ((data << 4) & 0xF0); // Send lower nibble
  PORTB |= (1 << EN);
  _delay_us(1);
  PORTB &= ~(1 << EN);
  _delay_ms(2);
}

// Print string to LCD
void lcdPrint(const char* str) {
  while (*str) {
    lcdData(*str++);
  }
}

// Clear LCD display
void lcdClear() {
  lcdCommand(0x01); // Clear display command
  _delay_ms(2);
}

// Initialize I2C in Slave Mode
void initI2C() {
  // Set the slave address
  TWAR = (I2C_ADDRESS << 1); // TWI (Two-Wire Interface) Address Register

  // Enable TWI and ACK
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);

  // Wait for data reception
  while (1) {
    // Wait for data reception to complete
    if (TWCR & (1 << TWINT)) {
      receiveI2CData();
    }
  }
}

// Handle I2C data reception
void receiveI2CData() {
  // Receive temperature
  temperature = TWDR; // Read temperature from TWI Data Register
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Clear interrupt flag
  while (!(TWCR & (1 << TWINT))); // Wait for next byte

  // Receive humidity
  humidity = TWDR; // Read humidity from TWI Data Register
  TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT); // Clear interrupt flag
}
