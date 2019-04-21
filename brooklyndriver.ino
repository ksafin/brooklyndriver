#include <Chrono.h>
#include <LightChrono.h>
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
uint8_t packet;

// Heartbeat
Chrono hb;
boolean hb_state = true;

// LED Constants
#define LED_OFF 0
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

  // Initialize SPI
  SPI.begin ();
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  // Wait for init packet from master
  // Init = 10101010
  setLED(LED_RED);
  uint8_t init = 0;
  while(init != 170) {
    waitForByte();
    init = Serial.read();
  }

  // Indicate handshake success for 2 seconds
  setLED(LED_BLUE);
  delay(2000);
  Serial.write(170);
  setLED(LED_OFF);
  hb.restart();
}

void setLED(uint8_t color) {
  if(color == LED_OFF) {
    digitalWrite(led_g, HIGH);
    digitalWrite(led_r, HIGH);
    digitalWrite(led_b, HIGH);
  } else if(color == LED_RED) {
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

void sendSPI(int cs, uint8_t c[], uint8_t buffsize) {
    digitalWrite(cs, LOW);
    SPI.transfer(c, buffsize);
    digitalWrite(cs, HIGH);
}

void waitForByte() {
  while (!Serial.available()) {};
}

void heartBeat() {
  if(hb.hasPassed(500)) {
    digitalWrite(led_g, hb_state);
    hb_state = !hb_state;
    hb.restart();
  }
}

// the loop routine runs over and over again forever:
void loop() {
  while(Serial.available()) {
    uint8_t checksum = 0;
    header = Serial.read();                       // Get new byte
    
    cid =  componentId_mask & header;  // Extract component ID (3 bit)
    //checksum = header;
    
    waitForByte();                                // Wait till next byte
    nparams = (uint8_t)Serial.read();                      // Get number of parameters
    //checksum = checksum + nparams * 10;

    //uint8_t buff[3+nparams];
    uint8_t buff[2+nparams];
    buff[0] = header;
    buff[1] = nparams;

    uint8_t byte;
    // Forward all parameters
    for(int i = 0; i < nparams; i++) {            
      waitForByte();   
      buff[2+i] = Serial.read();
      //checksum += byte % 10;
    }

    //sendSPI(ss[cid], buff, nparams+3);
    sendSPI(ss[cid], buff, nparams+2);
  }
  heartBeat();
}

