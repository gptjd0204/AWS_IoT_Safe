#include <Arduino.h>

#define CLOSE 0
#define OPEN 1

class Open {
  private:
    int pin;
    byte state;

  public:
    Open(int pin);
    void init();
    void open();
    void close();
    byte getState();
};