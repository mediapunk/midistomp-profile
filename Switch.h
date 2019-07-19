#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>

class Switch {
  
  bool    isPressed, isActivated, isLongActivated,
          sendPress, sendLongPress;
  uint8_t pin;
  long    pressTimer, longPressTimer;

  public:
    Switch(uint8_t);
    void run();
    bool pressed();
    bool longPressed();

};

#endif
