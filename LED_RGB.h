#include <Arduino.h>

#ifndef LED_RGB_H
#define LED_RGB_H

#define PIN_LATCH   9
#define PIN_CLOCK  12
#define PIN_DATA   11

#define LED_LEFT    0
#define LED_CENTER  4
#define LED_RIGHT   8

#define LED_FREQ   35

#define CHECK_BIT(var,pos)   ((var) &  (1<<(pos)))
#define TOGGLE_BIT(var,pos)  ((var) ^= (1<<(pos)))
#define SET_BIT(var,pos,val) ((var) = (((var) & ~(1<<(pos))) | ((val)<<(pos))))

typedef uint8_t RGB[3];

struct Color {
  unsigned long iter    = 0;
  unsigned long iters   = 0;
  uint8_t       points  = 1;
  uint8_t       target  = 0;
  uint8_t       actual  = 0;

  void run() {
    if (iter >= iters) {
      if      (target >= actual+points) actual += points;
      else if (target <= actual-points) actual -= points;
      else    actual = target;

      iter = 0;
    }
    else {
      iter++;
    }
  }

  unsigned long getFrames() {
    return (LED_FREQ*actual)/0xFF;
  }

  void set(uint8_t value, unsigned long time) {
    unsigned long difference;

    //iter   = 0;
    target = value;

    if (value > actual)
      difference = value - actual;
    else 
      difference = actual - value;

    if (difference > time) {
      iters  = 1;
      points = (uint8_t)(difference/time);
    }
    else if (time > difference) {
      points = 1;
      iters  = (uint8_t)(time/difference);
    }
    else {
      points = 1;
      iters  = 1;
    }
  }
};

struct Rgb {
  Color red, green, blue;
  Color         *c[3]        = {&red, &green, &blue};
  uint8_t       backup[3]    = {0,0,0};
  uint8_t       times        = 2;
  uint8_t       times_c      = 0;
  uint8_t       showoffEvent = 0;
  bool          isBlink      = false;
  bool          isShowoff    = false;
  bool          pong         = false;
  unsigned long delay        = 200;

  void set(uint8_t r, uint8_t g, uint8_t b, unsigned long time) {
    isBlink   = false;
    isShowoff = false;

    red.set(r,time);
    green.set(g,time);
    blue.set(b,time);

    backup[0] = r;
    backup[1] = g;
    backup[2] = b;
  }

  void blink(uint8_t _times, unsigned long _delay) {
    isBlink = true;
    delay   = _delay;
    times   = _times;
  }

  void showoff(uint8_t start) {
    if ((start <= 5) && (start > 0))
      showoffEvent = start;
    isShowoff = true;
  }

  bool isComplete() {
    if ((red.target   == red.actual)   &
        (green.target == green.actual) &
        (blue.target  == blue.actual))
      return true;
    return false;
  }

  void run() {
    red.run();
    green.run();
    blue.run();

    if (isBlink) {
      if (!pong) {
        if (isComplete()) {
          red.set(0, delay);
          green.set(0, delay);
          blue.set(0, delay);
          pong = true;
        }
      }
      else {
        if (isComplete()) {
          red.set(backup[0], delay);
          green.set(backup[1], delay);
          blue.set(backup[2], delay);
          pong = false;
          times_c++;
          if (times_c >= times) {
            isBlink = false;
            times_c = 0;
          }
        }
      }
    }
    else if (isShowoff) {
      switch(showoffEvent) {
        case 0:
          if (isComplete()) {
            red.set(100, 1000);
            green.set(0, 1000);
            blue.set(0, 1000);
            showoffEvent = 1;
          }
          break;

        case 1:
          if (isComplete()) {
            red.set(100, 1000);
            green.set(100, 1000);
            blue.set(0, 1000);
            showoffEvent = 2;
          }
          break;

        case 2:
          if (isComplete()) {
            red.set(0, 1000);
            green.set(100, 1000);
            blue.set(0, 1000);
            showoffEvent = 3;
          }
          break;

        case 3:
          if (isComplete()) {
            red.set(0, 1000);
            green.set(100, 1000);
            blue.set(100, 1000);
            showoffEvent = 4;
          }
          break;

        case 4:
          if (isComplete()) {
            red.set(0, 1000);
            green.set(0, 1000);
            blue.set(100, 1000);
            showoffEvent = 5;
          }
          break;

        case 5:
          if (isComplete()) {
            red.set(100, 1000);
            green.set(0, 1000);
            blue.set(100, 1000);
            showoffEvent = 0;
          }
          break;
      }
    }
    else {
      pong = false;
    }
  }
};

class LED_RGB {
  static long    bits;
  static uint8_t latch, clock, data;

  uint8_t start_bit, frame;
  Rgb     rgb;

  public:
    static void init(uint8_t, uint8_t, uint8_t);
    static void loop();

    LED_RGB(uint8_t);
    void run();
    void set(uint8_t, uint8_t, uint8_t, unsigned long);
    bool isComplete();
    void blink(uint8_t, unsigned long);
    void showoff(uint8_t);
};

#endif
