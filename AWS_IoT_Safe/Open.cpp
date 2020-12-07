#include "Open.h"

Open::Open(int pin) {
  this->pin = pin;
  init();
}
void Open::init() {
  //pinMode(pin, OUTPUT);
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
