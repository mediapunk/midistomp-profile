#ifndef SWITCH_H
#define SWITCH_H

#include <Arduino.h>

#define DEBOUNCE_TIME        100
#define LONG_PRESS_TIME      300

#define PIN_LEFT               2
#define PIN_CENTER             3
#define PIN_RIGHT              4

#define LISTENING              0
#define DEBOUNCE               1
#define BTN_CALC               2
#define BTN_PRESS              3
#define BTN_LONG_PRESS         4
#define BTN_LONG_PRESS_RESET   5

struct Switch {
  uint8_t pin             = 0;
  uint8_t e               = 0;        // Event
  long    last_hw_press_t = 0;        // Timer
  bool    isPressed       = false;
  bool    isLongPressed   = false;
  bool    initialize      = false;

  Switch(uint8_t pin) {
    this->pin = pin;
  }

  void loop(long timer) {
    switch(e) {
      case LISTENING:
        isPressed     = false;
        isLongPressed = false;

        if (!initialize) {
          last_hw_press_t = timer;
          initialize      = true;
        } else {
          if (timer >= last_hw_press_t+DEBOUNCE_TIME) {
            if (!digitalRead(pin)) {
              last_hw_press_t = timer;
              e               = DEBOUNCE;
            }
          }
        }
        break;

      case DEBOUNCE:
        if (timer >= last_hw_press_t+DEBOUNCE_TIME) e = BTN_CALC;
        break;

      case BTN_CALC:
        if (!digitalRead(pin)) {
          if (timer >= last_hw_press_t+LONG_PRESS_TIME) e = BTN_LONG_PRESS;
        } else {
          e = BTN_PRESS;
        }
        break;

      case BTN_PRESS:
        isPressed  = true;
        e          = LISTENING;
        initialize = false;
        break;

      case BTN_LONG_PRESS:
        isLongPressed = true;
        e             = BTN_LONG_PRESS_RESET;
        break;

      case BTN_LONG_PRESS_RESET:
        isLongPressed = false;
        if (digitalRead(pin)) {
          e          = LISTENING;
          initialize = false;
        }
        break;

      default:
        e          = LISTENING;
        initialize = false;
        break;
    }
  }
};

#endif
