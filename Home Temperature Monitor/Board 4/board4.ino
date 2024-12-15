#include <Wire.h>

const int buttonPin = 4;
volatile int pressCount = 0;
int oldValue = HIGH; // default/idle value for pin 4 is high.

void setup() 
{
  Serial.begin(115200);
  Wire.begin(3);
  Wire.onRequest(sendButtonCount);
  Serial.println("Press count");

  // Initialize the pin for reading the button.
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() 
{
  // Read the value of pin 4.
  int newValue = digitalRead(buttonPin);

  // Check if the value was changed,
  // by comparing it with the previous value.
  if(newValue != oldValue)
  {
    if(newValue == LOW)
    {
      Serial.println(pressCount);
      
      pressCount++;
      if (pressCount>3){
        pressCount = 0;
      }
    }
    // Remember the value for the next time.
    oldValue = newValue;
  }

  // Slow down the sketch.
  // Also for debouncing the button.
  delay(100);
}

void sendButtonCount(){
  Wire.write(pressCount);
  Serial.println("Request Made");
  Serial.println(pressCount);
}
