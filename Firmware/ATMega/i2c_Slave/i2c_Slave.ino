// Include the Wire library for I2C
#include <Wire.h>

constexpr unsigned long FORCE_PWR_OFF_TIMEOUT = 5000; // in ms
 
// LED on pin 13
const int ledPin = 12; 

const unsigned PWR_BUTTON_PIN = 0;
const unsigned JOY_BUTTONS_PINS[] = {
  1, // BUTTON A
  2, // BUTTON B
  3, // DHAT UP
  4, // DHAT DOWN
  5, // DHAT LEFT
  6, // DHAT RIGHT
  7, // SHOULDER LEFT
  8  // SHOULDER RIGHT
};

const size_t JOY_BUTTONS_PINS_LEN = 8;

unsigned long press_time = 0;
 
void setup() {
  // First thing: put pwr pin to 5V
  pinMode(PWR_BUTTON_PIN, INPUT_PULLUP);

  // Join I2C bus as slave with address 8
  Wire.begin(0x8);
  
  // I²C callbacks    
  Wire.onRequest(requestEvent);           
  Wire.onReceive(receiveEvent);

  // Setup input pins
  for(size_t i = 0; i < JOY_BUTTONS_PINS_LEN; ++i)
    pinMode(JOY_BUTTONS_PINS[i], INPUT_PULLUP);
  
  // Setup pin 13 as output and turn LED off
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
}

void power_off() {
  pinMode(PWR_BUTTON_PIN, OUTPUT);
  digitalWrite(PWR_BUTTON_PIN, LOW);
}
 
// Function that executes whenever data is received from master
void receiveEvent(int howMany) {
  while (Wire.available()) {
    uint8_t packet = Wire.read();
    digitalWrite(ledPin, packet);

    if(packet & 0x01 == 0)
      power_off();
  }
}

void requestEvent() {
  uint16_t packet;
  unsigned mask = 0b10;

  packet = digitalRead(PWR_BUTTON_PIN) == LOW ? 1 : 0;
  
  for(size_t i = 0; i < JOY_BUTTONS_PINS_LEN; ++i, mask <<= 1)
    packet |= digitalRead(JOY_BUTTONS_PINS[i]) == LOW ? mask : 0;
  
  Wire.write((char*)&packet, sizeof(uint16_t));
}

void loop() {
  if(press_time == 0 and digitalRead(PWR_BUTTON_PIN) == LOW)
    press_time = millis();
  else if (digitalRead(PWR_BUTTON_PIN) == HIGH)
    press_time = 0;
  else if(millis() - press_time >= FORCE_PWR_OFF_TIMEOUT)
    power_off();

  delay(100);
}