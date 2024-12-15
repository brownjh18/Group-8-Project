// Define LCD control pins
#define RS 0  // Register Select pin on PORTB, PB0
#define EN 1  // Enable pin on PORTB, PB1
#define D4 2  // Data pin 4 on PORTB, PB2
#define D5 3  // Data pin 5 on PORTB, PB3
#define D6 4  // Data pin 6 on PORTB, PB4
#define D7 5  // Data pin 7 on PORTB, PB5

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
void delayMs(uint16_t ms);         // Custom millisecond delay function

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
    delayMs(1000);
  }

  return 0;
}

// Initialize the LCD
void initLCD() {
  // Set LCD pins as output
  *(volatile uint8_t *)0x24 |= (1 << RS) | (1 << EN) | (1 << D4) | (1 << D5) | (1 << D6) | (1 << D7); // DDRB

  // LCD initialization sequence
  delayMs(20);              // Wait for LCD to power up
  lcdCommand(0x03);         // Set 8-bit mode
  delayMs(5);
  lcdCommand(0x03);         // Repeat
  delayMs(1);
  lcdCommand(0x03);         // Repeat
  lcdCommand(0x02);         // Set 4-bit mode
  lcdCommand(0x28);         // 2-line, 5x7 matrix
  lcdCommand(0x0C);         // Display ON, Cursor OFF
  lcdCommand(0x06);         // Entry mode, auto-increment
  lcdCommand(0x01);         // Clear display
}

// Send command to LCD
void lcdCommand(uint8_t cmd) {
  *(volatile uint8_t *)0x25 &= ~(1 << RS); // PORTB &= ~RS (RS = 0 for command)

  // Send higher nibble
  *(volatile uint8_t *)0x25 = (*(volatile uint8_t *)0x25 & 0x0F) | (cmd & 0xF0); // Update PORTB
  *(volatile uint8_t *)0x25 |= (1 << EN); // PORTB |= EN
  delayMs(1);
  *(volatile uint8_t *)0x25 &= ~(1 << EN); // PORTB &= ~EN

  // Send lower nibble
  *(volatile uint8_t *)0x25 = (*(volatile uint8_t *)0x25 & 0x0F) | ((cmd << 4) & 0xF0);
  *(volatile uint8_t *)0x25 |= (1 << EN); // PORTB |= EN
  delayMs(1);
  *(volatile uint8_t *)0x25 &= ~(1 << EN); // PORTB &= ~EN

  delayMs(2);
}

// Send data to LCD
void lcdData(uint8_t data) {
  *(volatile uint8_t *)0x25 |= (1 << RS); // PORTB |= RS (RS = 1 for data)

  // Send higher nibble
  *(volatile uint8_t *)0x25 = (*(volatile uint8_t *)0x25 & 0x0F) | (data & 0xF0); // Update PORTB
  *(volatile uint8_t *)0x25 |= (1 << EN); // PORTB |= EN
  delayMs(1);
  *(volatile uint8_t *)0x25 &= ~(1 << EN); // PORTB &= ~EN

  // Send lower nibble
  *(volatile uint8_t *)0x25 = (*(volatile uint8_t *)0x25 & 0x0F) | ((data << 4) & 0xF0);
  *(volatile uint8_t *)0x25 |= (1 << EN); // PORTB |= EN
  delayMs(1);
  *(volatile uint8_t *)0x25 &= ~(1 << EN); // PORTB &= ~EN

  delayMs(2);
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
  delayMs(2);
}

// Custom millisecond delay function
void delayMs(uint16_t ms) {
  for (uint16_t i = 0; i < ms; i++) {
    for (uint16_t j = 0; j < 1000; j++) {
      asm volatile("nop");
    }
  }
}

// Initialize I2C in Slave Mode
void initI2C() {
  // Set the slave address
  *(volatile uint8_t *)0xB8 = (I2C_ADDRESS << 1); // TWAR (TWI Address Register)

  // Enable TWI and ACK
  *(volatile uint8_t *)0xBC = (1 << 2) | (1 << 6) | (1 << 7); // TWCR (TWI Control Register)

  // Wait for data reception
  while (1) {
    // Wait for data reception to complete
    if (*(volatile uint8_t *)0xBC & (1 << 7)) { // Check TWINT
      receiveI2CData();
    }
  }
}

// Handle I2C data reception
void receiveI2CData() {
  // Receive temperature
  temperature = *(volatile uint8_t *)0xBB; // Read temperature from TWDR (TWI Data Register)
  *(volatile uint8_t *)0xBC = (1 << 2) | (1 << 6) | (1 << 7); // Clear interrupt flag
  while (!(*(volatile uint8_t *)0xBC & (1 << 7))); // Wait for next byte

  // Receive humidity
  humidity = *(volatile uint8_t *)0xBB; // Read humidity from TWDR
  *(volatile uint8_t *)0xBC = (1 << 2) | (1 << 6) | (1 << 7); // Clear interrupt flag
}
