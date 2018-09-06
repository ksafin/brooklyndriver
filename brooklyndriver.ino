#include <SPI.h>
#include "pins_arduino.h"

// Slave Select Pins
uint8_t ss[] = {A1, A0, 0, 1, 10, 9, 12, 4};

// Reset Pins
uint8_t reset[] = {A2, A3, 3, 2, 13, 5, 8, 6};

// State pins
uint8_t batt = A4;
uint8_t led_g = A5;
uint8_t led_r = 7;
uint8_t led_b = 11;

// Bit Masks
// Masks for extracting relevant data
#define componentId_mask    0b00000111

// Current Packet Data
uint8_t cid;      // Component ID
uint8_t nparams;  // Num Parameters
uint8_t header;

// LED Constants
#define LED_RED 1
#define LED_GREEN 2
#define LED_BLUE 3

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(1000000);

  // Initialize Slave Selects & Resets
  for(int i = 0; i < 8; i++) {
    pinMode(ss[i], OUTPUT);
    pinMode(reset[i], OUTPUT);
    digitalWrite(ss[i], HIGH);
    digitalWrite(reset[i], HIGH);
  }

  // Initialize LED
  pinMode(led_g, OUTPUT);
  pinMode(led_r, OUTPUT);
  pinMode(led_b, OUTPUT);
  setLED(LED_GREEN);

  // Initialize SPI
  SPI.begin ();
  SPI.setClockDivider(SPI_CLOCK_DIV8);
}

void setLED(uint8_t color) {
  if(color == LED_RED) {
    digitalWrite(led_g, HIGH);
    digitalWrite(led_r, LOW);
    digitalWrite(led_b, HIGH);
  } else if (color == LED_GREEN) {
    digitalWrite(led_g, LOW);
    digitalWrite(led_r, HIGH);
    digitalWrite(led_b, HIGH);
  } else if (color == LED_BLUE) {
    digitalWrite(led_g, HIGH);
    digitalWrite(led_r, HIGH);
    digitalWrite(led_b, LOW);
  }
}

void sendSPI(int cs, char c) {
    digitalWrite(cs, LOW);
    SPI.transfer(c);
    digitalWrite(cs, HIGH);
}

void waitForByte() {
  while (!Serial.available()) {};
}

// the loop routine runs over and over again forever:
void loop() {
  while(Serial.available()) {
    header = Serial.read();                       // Get new byte
    cid =  componentId_mask & header;             // Extract component ID (3 bit)
    sendSPI(ss[cid], header);                     // Transmit to proper uC
    
    waitForByte();                                // Wait till next byte
    nparams = Serial.read();                      // Get number of parameters
    sendSPI(ss[cid], nparams);                    // Transmit nparams to proper uC

    // Forward all parameters
    for(int i = 0; i < nparams; i++) {            
      waitForByte();                              // Wait till next byte
      sendSPI(ss[cid], Serial.read());            // Send to proper uC
    }
  }
}

