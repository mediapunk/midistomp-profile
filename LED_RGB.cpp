#include "LED_RGB.h"

LED_RGB::LED_RGB(uint8_t _start_bit) {
  start_bit = _start_bit;
  frame     = 0;
}

void LED_RGB::run() {
  for (int i=0; i<3; i++) {
    if (frame < rgb.c[i]->getFrames()) {
      SET_BIT(bits, start_bit+i, 1);
    }
    else {
      SET_BIT(bits, start_bit+i, 0);
    }
  }

  rgb.run();

  if (frame<LED_FREQ-1) frame++;
  else                  frame = 0;
}

void LED_RGB::set(uint8_t r, uint8_t g, uint8_t b, unsigned long time) {
  rgb.set(r,g,b,time);
}

bool LED_RGB::isComplete() {
  if (rgb.isComplete())
    return true;
  return false;
}

void LED_RGB::blink(uint8_t times, unsigned long delay) {
  rgb.blink(times, delay);
}

void LED_RGB::showoff(uint8_t start) { rgb.showoff(start); }


// -- Static -- 
long    LED_RGB::bits  = 0;
uint8_t LED_RGB::latch = PIN_LATCH;
uint8_t LED_RGB::clock = PIN_CLOCK;
uint8_t LED_RGB::data  = PIN_DATA;

void LED_RGB::init(uint8_t _latch, uint8_t _clock, uint8_t _data) {
  latch = _latch;
  clock = _clock;
  data  = _data;
}

void LED_RGB::loop() {
  digitalWrite(latch, LOW);
  shiftOut(data, clock, MSBFIRST, bits >> 8);
  shiftOut(data, clock, MSBFIRST, bits &  0xFF);
  digitalWrite(latch, HIGH);
}
// -- End Static -- 
