#define BUTTON_PIN PB0 // Button connected to pin B0
#define I2C_SCL PC5    // I2C clock on pin C5
#define I2C_SDA PC4    // I2C data on pin C4

// Variables to track system state
volatile uint8_t systemOn = 1;

// Function prototypes
void initGPIO();            // Initialize GPIO for button
void initI2C();             // Initialize I2C in master mode
void toggleSystemState();   // Toggle system state on button press
void requestData(uint8_t address, float* temp, float* hum); // Request data from slave
void sendData(uint8_t address, float temp, float hum);      // Send data to slave

int main(void) {
    // Initialize GPIO and I2C
    initGPIO();
    initI2C();

    float temperature = 0.0;
    float humidity = 0.0;

    while (1) {
        // Check button state and toggle system state if needed
        if (!(PINB & (1 << BUTTON_PIN))) { // Button pressed
            _delay_ms(200); // Debounce delay
            toggleSystemState();
        }

        if (systemOn) {
            // Request data from Slave 2 (DHT11 readings)
            requestData(2, &temperature, &humidity);

            // Forward data to Slave 1 for display
            sendData(1, temperature, humidity);

            // Forward data to Slave 3 for storage
            sendData(3, temperature, humidity);

            // Wait 2 seconds before the next cycle
            _delay_ms(2000);
        }
    }

    return 0;
}

void initGPIO() {
    // Configure BUTTON_PIN as input with pull-up resistor
    DDRB &= ~(1 << BUTTON_PIN); // Set BUTTON_PIN as input
    PORTB |= (1 << BUTTON_PIN); // Enable pull-up resistor
}

void initI2C() {
    // Set clock frequency for I2C (prescaler = 1, SCL = 100kHz for 16MHz CPU)
    TWSR = 0x00;               // Prescaler value = 1
    TWBR = 72;                 // Bit rate register (calculated for 100kHz)
    TWCR = (1 << TWEN);        // Enable TWI
}

void toggleSystemState() {
    systemOn = !systemOn;
    if (systemOn) {
        // Debug message for system ON
        // Replace with UART output if required
    } else {
        // Debug message for system OFF
        // Replace with UART output if required
    }
}

void requestData(uint8_t address, float* temp, float* hum) {
    uint8_t tempData[4] = {0};
    uint8_t humData[4] = {0};

    // Send START condition
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));

    // Send slave address with read bit
    TWDR = (address << 1) | 0x01;
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));

    // Read temperature (4 bytes)
    for (uint8_t i = 0; i < 4; i++) {
        TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
        while (!(TWCR & (1 << TWINT)));
        tempData[i] = TWDR;
    }

    // Read humidity (4 bytes)
    for (uint8_t i = 0; i < 4; i++) {
        TWCR = (1 << TWEN) | (1 << TWEA) | (1 << TWINT);
        while (!(TWCR & (1 << TWINT)));
        humData[i] = TWDR;
    }

    // Send STOP condition
    TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);

    // Convert data to float
    memcpy(temp, tempData, 4);
    memcpy(hum, humData, 4);
}

void sendData(uint8_t address, float temp, float hum) {
    uint8_t tempData[4];
    uint8_t humData[4];

    // Convert float to byte array
    memcpy(tempData, &temp, 4);
    memcpy(humData, &hum, 4);

    // Send START condition
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));

    // Send slave address with write bit
    TWDR = (address << 1);
    TWCR = (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));

    // Send temperature (4 bytes)
    for (uint8_t i = 0; i < 4; i++) {
        TWDR = tempData[i];
        TWCR = (1 << TWEN) | (1 << TWINT);
        while (!(TWCR & (1 << TWINT)));
    }

    // Send humidity (4 bytes)
    for (uint8_t i = 0; i < 4; i++) {
        TWDR = humData[i];
        TWCR = (1 << TWEN) | (1 << TWINT);
        while (!(TWCR & (1 << TWINT)));
    }

    // Send STOP condition
    TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);
}
