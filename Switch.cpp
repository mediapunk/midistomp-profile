#include "Switch.h"

Switch::Switch(uint8_t pin) {
  this->pin       = pin;
  isPressed       = false;
  isActivated     = false;
  isLongActivated = false;
  sendPress       = false;
  sendLongPress   = false;
  pressTimer      = 0;
  longPressTimer  = 0;
}

void Switch::run() {
  if (!digitalRead(pin)) {  // If button is pressed
    if (!isPressed) {
      isPressed  = true;  // Register a hardware press is in process
      pressTimer = millis();
    }
    else {
      /* 
        If it has been over so many ms since button was
        pressed down.  This is to prevent accidental
        triggering from poor quality switches.
      */
      if (!isActivated) {
        if (millis()-pressTimer > DEBOUNCE_TIME) {
          longPressTimer = millis();
          isActivated    = true;  // Button now can register a real press
        }
      }

      /*
        If it has been over a certain ms since button was
        pressed down, then activate a long press 
        action.
      */
      else {
        if (millis()-longPressTimer > LONGPRESS_TIME) {
          if (!isLongActivated) {
            isLongActivated = true;
            sendLongPress   = true;
          }
          else {
            sendLongPress = false;
          }
        }
      }
    }
  }
  else {
    if (isPressed) isPressed = false;

    if (isActivated) {
      isActivated = false;

      if (isLongActivated) {
        isLongActivated = false;
        sendLongPress   = false;
      }
      else {
        sendPress = true;
      }
    }
    else {
      if (sendPress)     sendPress     = false;
      if (sendLongPress) sendLongPress = false;
    }
  }
}

bool Switch::pressed()     { return sendPress; }
bool Switch::longPressed() { return sendLongPress; }


