#include <Arduino.h>

#define SHOCK_OFF 0
#define SHOCK_ON 1
#define SHOCK_STOP 0
#define SHOCK_RUN 1

class Shock {
  private:
    int pin;
    byte state;
	byte runState;

  public:
    Shock(int pin);
    void init();
    void on();
    void off();
	void runShock();
	void stopShock();
    byte getState();
	byte getRunState();
};
