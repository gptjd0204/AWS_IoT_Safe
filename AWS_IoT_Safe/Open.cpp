#include "Open.h"

Open::Open(int pin) {
  // Use 'this->' to make the difference between the
  // 'pin' attribute of the class and the 
  // local variable 'pin' created from the parameter.
  this->pin = pin;
  init();
}
void Open::init() {
  pinMode(pin, OUTPUT);
  // Always try to avoid duplicate code.
  // Instead of writing digitalWrite(pin, LOW) here,
  // call the function off() which already does that
  closeSafe();
  state = CLOSE;
}

void Open::openSafe() {
  state = OPEN;
}

void Open::closeSafe() {
  state = CLOSE;
}

byte Open::getState() {
  return state;
}
