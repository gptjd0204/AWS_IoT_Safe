#include "Shock.h"

Shock::Shock(int pin) {
  // Use 'this->' to make the difference between the
  // 'pin' attribute of the class and the 
  // local variable 'pin' created from the parameter.
  this->pin = pin;
  init();
}
void Shock::init() {
  pinMode(pin, OUTPUT);
  // Always try to avoid duplicate code.
  // Instead of writing digitalWrite(pin, LOW) here,
  // call the function off() which already does that
  off();
  state = SHOCK_OFF;
  runState = SHOCK_STOP;
}

void Shock::on() {
  state = SHOCK_ON;
}

void Shock::off() {
  state = SHOCK_OFF;
}

void Shock::run() {
  runState = SHOCK_RUN;
}

void Shock::stop() {
  runState = SHOCK_STOP;
}

byte Shock::getState() {
  return state;
}

byte Shock::getRunState() {
  return runState;
}
