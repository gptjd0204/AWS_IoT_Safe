#include "Led.h"

Led::Led(int pin) {
  // Use 'this->' to make the difference between the
  // 'pin' attribute of the class and the 
  // local variable 'pin' created from the parameter.
  this->pin = pin;
  init();
}
void Led::init() {
  pinMode(pin, OUTPUT);
  // Always try to avoid duplicate code.
  // Instead of writing digitalWrite(pin, LOW) here,
  // call the function off() which already does that
  off();
  runOff();
  state = LED_OFF;
}
void Led::on() {
  state = LED_ON;
  for(int i = 0; i < 3; i++){
    digitalWrite(pin, HIGH);
    delay(1000);
    digitalWrite(pin, LOW);
    delay(1000);
  }
}


void Led::off() {
  digitalWrite(pin, LOW);
  state = LED_OFF;
}

void Led::runOn() {
    digitalWrite(pin, HIGH);
}

void Led::runOff() {
  digitalWrite(pin, LOW);
}


byte Led::getState() {
  return state;
}
