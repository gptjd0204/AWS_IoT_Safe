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
    void openSafe();
    void closeSafe();
    byte getState();
};
