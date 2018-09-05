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

// Test Packets for Motor Control
// 00001000
// 00100000
// 11111111

// Bit Masks
// Masks for extracting relevant data
#define subcomponentId_mask 0b00000001
#define componentId_mask    0b00001110
#define functionId_mask     0b11110000
#define parameterQty_mask   0b11100000
#define cardType_mask       0b00011111

// Packet Definition
// Byte 1 - ffffcccs 
// Byte 2 - pppttttt
// f - Function ID
// c - Component ID
// s - SubComponent ID
// p - Num Parameters
// t - Card Type


// Current Packet Data
uint8_t fid;      // Function ID
uint8_t cid;      // Component ID
uint8_t scid;     // Subcomponent ID
uint8_t tid;      // Type ID
uint8_t nparams;  // Num Parameters
uint8_t params[30];
uint8_t header;
uint8_t packet;

// LED Constants
#define LED_RED 1
#define LED_GREEN 2
#define LED_BLUE 3

// the setup routine runs once when you press reset:
void setup() {                
  Serial.begin(115200);

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

// the loop routine runs over and over again forever:
void loop() {
  while(Serial.available()) {
    setLED(LED_BLUE);
    header = Serial.read();                       // Get new byte
    Serial.write(header);
    cid =  (uint8_t)((componentId_mask & header) >> 1);       // Extract component ID (3 bit)
    scid = (uint8_t)(header & subcomponentId_mask);          // Extract subcomponent ID (1 bit)
    fid =  (uint8_t)((functionId_mask & header) >> 4);        // Extract function ID (4 bit)
    Serial.write(cid);
    Serial.write(scid);
    Serial.write(fid);
    while (!Serial.available()) {};               // Wait till next byte
    header = Serial.read();                       // Get second byte
    nparams = (header & parameterQty_mask) >> 5;  // Get parameter quantity (3 bit)
    tid = header & cardType_mask;                 // Get Card Type ID (5 bit)
    Serial.write(nparams);
    Serial.write(tid);
    for(int i = 0; i < nparams; i++) {            // Get all parameters
      while (!Serial.available()) {};             // Wait till next byte
      params[i] = Serial.read();
    }

    packet = (fid << 4) + (nparams << 1) + scid;  //ffffppps
    sendSPI(ss[cid], packet);                     // Send header
    for (int i = 0; i < nparams; i++) {           // Send all parameters
      sendSPI(ss[cid], params[i]); 
    }
  }
}

