#include "Shock.h"

Shock::Shock(int pin) {
  this->pin = pin;
  init();
}
void Shock::init() {
  pinMode(pin, INPUT);
  off();
  stopShock();
  state = SHOCK_OFF;
  runState = SHOCK_STOP;
}

void Shock::on() {
  state = SHOCK_ON;
}

void Shock::off() {
  state = SHOCK_OFF;
}

void Shock::runShock() {
  runState = SHOCK_RUN;
}

void Shock:: stopShock(){
  runState = SHOCK_STOP;
}

byte Shock::getState() {
  return state;
}

byte Shock::getRunState() {
  return runState;
}
