#include "Switch.h"
#include "LED_RGB.h"


// -- DEFINITIONS --
#ifndef FALSE
#define FALSE               0
#endif
#ifndef TRUE
#define TRUE                1
#endif
#define FALSE2              2

#define MIDI_CC          0xB0
#define MIDI_PC          0xC0
#define MIDI_NOTE_OFF    0x80
#define MIDI_NOTE_ON     0x90
#define MIDI_NOTE_SNAP     69

#define FS_EXP1             1
#define FS_EXP2             2
#define FS1                49
#define FS2                50
#define FS3                51
#define FS4                52
#define FS5                53
#define FS6                48
#define FS_MODE            71
#define FS_MODE_SCROLL      1
#define FS_MODE_STOMP       0

#define LONG_PRESS_TIME   200

#define PIN_LATCH           9
#define PIN_CLOCK          12
#define PIN_DATA           11

#define PIN_LEFT            2
#define PIN_CENTER          3
#define PIN_RIGHT           4

#define LED_LEFT            1
#define LED_CENTER          2
#define LED_RIGHT           3

#define MENU_PRE            0
#define MENU_PRE_FADE_IN    1
#define MENU_PRE_FADE_OUT   2
#define MENU_MAIN           3
#define MENU_HIPBOX         4
#define MENU_CHANGE_PRESET  5
#define MENU_CHANGE_SNAP    6

#define CHECK_BIT(var,pos)  ((var) &  (1<<(pos)))
#define TOGGLE_BIT(var,pos) ((var) ^= (1<<(pos)))
// -- END DEFINITIONS --


class FootSwitch {
  bool    state;
  uint8_t note;

  public:
    FootSwitch(uint8_t note) {
      state      = false;
      this->note = note;
    }

    void send() {
      Serial.write(MIDI_CC);
      Serial.write(note);
      Serial.write(state ? 127 : 0);
    }

    bool get()           { return state; }
    void set(bool state) { this->state = state; }

    void toggle() {
      if (state) set(false);
      else       set(true);
      send();
    }
};

FootSwitch fs4(FS4);
FootSwitch fs5(FS5);
FootSwitch fs6(FS6);

struct Snap {
  bool _fs4 = FALSE, _fs5 = FALSE, _fs6 = FALSE;

  void resetFootSwitches() {
    _fs4 = FALSE;
    _fs5 = FALSE;
    _fs6 = FALSE;
  }

  void setSwitches() {
    fs4.set(_fs4);
    fs5.set(_fs5);
    fs6.set(_fs6);
  }
};

class Preset {
  uint8_t presetNum, snapNum;
  Snap    snap_list[3];

  public:
    Snap *snap_p;

    Preset() {
      presetNum = 0;
      snapNum   = 0;
      snap_p    = &snap_list[snapNum];
    }

    void loadPreset() {
      Serial.write(MIDI_PC);
      Serial.write(presetNum);
    }
    void set(uint8_t presetNum) {
      this->presetNum = presetNum;
      loadPreset();
      resetSnaps();
      snapNum = 0;
    }
    void    next()     { if (presetNum < 126) set(presetNum+1); }
    void    previous() { if (presetNum > 0)   set(presetNum-1); }
    uint8_t get()      { return presetNum; }


    void resetSnaps() {
      fs4.set(FALSE);
      fs5.set(FALSE);
      fs6.set(FALSE);
      for (int i=0; i<3; i++) snap_list[i].resetFootSwitches();
    }
    void loadSnap() {
      Serial.write(MIDI_CC);
      Serial.write(MIDI_NOTE_SNAP);
      Serial.write(snapNum);
    }
    void setSnap(uint8_t snapNum) {
      if (snapNum > 2) snapNum = 2;
      this->snapNum = snapNum;
      snap_p = &snap_list[snapNum];
      snap_p->setSwitches();
      loadSnap();
    }
    void    nextSnap()     { if (snapNum < 2) setSnap(snapNum+1); }
    void    previousSnap() { if (snapNum > 0) setSnap(snapNum-1); }
    uint8_t getSnap()      { return snapNum; }
};

void sendMidi(uint8_t command, uint8_t note, uint8_t velocity) {
  Serial.write(command);
  Serial.write(note);
  Serial.write(velocity);
}

void sendMidi(uint8_t command, uint8_t note) {
  Serial.write(command);
  Serial.write(note);
}


// -- VARIABLES --
uint8_t event, initialize;
long    timer;

Switch lsw(PIN_LEFT);
Switch csw(PIN_CENTER);
Switch rsw(PIN_RIGHT);

LED_RGB led1(0);
LED_RGB led2(4);
LED_RGB led3(8);

Preset preset;
// -- END VARIABLES --


void jumpTo(uint8_t newEvent) {
  event      = newEvent;
  initialize = FALSE;
}


void setup() {
  Serial.begin(31250);
  LED_RGB::init(PIN_LATCH, PIN_CLOCK, PIN_DATA);
  event = 0;
  timer = 0;

  // BOARD LED
  pinMode(13,         OUTPUT);

  // FOOT SWITCHES
  pinMode(PIN_LEFT,   INPUT_PULLUP);
  pinMode(PIN_CENTER, INPUT_PULLUP);
  pinMode(PIN_RIGHT,  INPUT_PULLUP);

  // Shift Registers
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_DATA,  OUTPUT);
}

void loop() {
  LED_RGB::loop();
  led1.run();
  led2.run();
  led3.run();

  lsw.run();
  csw.run();
  rsw.run();

  switch(event) {
    case MENU_PRE:
      if (!initialize) {
        led1.showoff(2);
        led2.showoff(1);
        led3.showoff(0);
        initialize = TRUE;
      }
      else {
        if (lsw.longPressed() || rsw.longPressed() || csw.longPressed()) {
          sendMidi(MIDI_CC, FS_MODE, FS_MODE_STOMP);
          preset.set(1);
          preset.set(0);
          jumpTo(MENU_PRE_FADE_IN);
        }
      }
      break;

    case MENU_PRE_FADE_IN:
      led1.set(0,255,0, 1000);
      led2.set(0,255,0, 1000);
      led3.set(0,255,0, 1000);
      timer = millis();
      jumpTo(MENU_PRE_FADE_OUT);
      break;

    case MENU_PRE_FADE_OUT:
      if (led1.isComplete()) {
        if (millis()-timer>1300) {
          led1.set(0,0,0, 1000);
          led2.set(0,0,0, 1000);
          led3.set(0,0,0, 1000);
          jumpTo(MENU_MAIN);
        }
      }
      break;

    case MENU_MAIN:
      if (!initialize && led1.isComplete()) {
        led1.set(fs4.get() ? 255 : 0,0,0, 200);
        led2.set(fs5.get() ? 255 : 0,0,0, 200);
        led3.set(fs6.get() ? 255 : 0,0,0, 200);
        initialize = TRUE;
      }
      else {
        if (lsw.pressed()) {
          fs4.toggle();
          preset.snap_p->_fs4 = fs4.get();
          led1.set(fs4.get() ? 255 : 0,0,0, 50);
        }
        if (csw.pressed()) {
          fs5.toggle();
          preset.snap_p->_fs5 = fs5.get();
          led2.set(fs5.get() ? 255 : 0,0,0, 50);
        }
        if (rsw.pressed()) {
          fs6.toggle();
          preset.snap_p->_fs6 = fs6.get();
          led3.set(fs6.get() ? 255 : 0,0,0, 50);
        }

        if (lsw.longPressed()) {
          jumpTo(MENU_HIPBOX);
        }
        if (csw.longPressed()) {
          jumpTo(MENU_CHANGE_SNAP);
        }
        if (rsw.longPressed()) {
          jumpTo(MENU_CHANGE_PRESET);
        }
      }
      break;

    case MENU_HIPBOX:
      if (!initialize) {
        if (preset.get() > 41) {
          led1.set(50,0,0, 200);
          led2.set(50,0,0, 200);
          led3.set(50,0,0, 200);
        }
        else {
          led1.set(255,20,0, 200);
          led2.set(255,20,0, 200);
          led3.set(255,20,0, 200);
        }
        initialize = TRUE;
      }
      else {
        if (preset.get() <= 41) {
          if (lsw.pressed()) {
            sendMidi(MIDI_NOTE_ON, (preset.get()*3), 127);
            led1.blink(2, 50);
          }
          if (csw.pressed()) {
            sendMidi(MIDI_NOTE_ON, ((preset.get()*3)+1), 127);
            led2.blink(2, 50);
          }
          if (rsw.pressed()) {
            sendMidi(MIDI_NOTE_ON, ((preset.get()*3)+2), 127);
            led3.blink(2, 50);
          }
        }
        if (lsw.longPressed()) {
          preset.resetSnaps();
          jumpTo(MENU_PRE);
        }
        if (csw.longPressed() || rsw.longPressed()) {
          jumpTo(MENU_MAIN);
        }
      }
      break;

    case MENU_CHANGE_PRESET:
      if (!initialize) {
        led1.set(255,135,0, 200);
        led2.set(  0,  0,0, 200);
        led3.set(255,135,0, 200);
        initialize = TRUE;
      }
      else {
        if (lsw.pressed()) {
          if (preset.get() == 0) preset.set(125);
          else preset.previous();
          led1.blink(2, 50);
        }
        if (csw.pressed()) {
          jumpTo(MENU_MAIN);
        }
        if (rsw.pressed()) {
          if (preset.get() == 125) preset.set(0);
          else preset.next();
          led3.blink(2, 50);
        }

        if (lsw.longPressed()) {
          preset.set(preset.get() > 20 ? preset.get()-20 : 0);
          led1.blink(2, 100);
        }
        if (rsw.longPressed()) {
          preset.set(preset.get() < 105 ? preset.get()+20 : 125);
          led3.blink(2, 100);
        }
      }
      break;

    case MENU_CHANGE_SNAP:
      if (!initialize) {
        if (preset.getSnap() == 0) led1.set(255,255,255,200);
        else                       led1.set(  0,  0,  0,200);
        if (preset.getSnap() == 1) led2.set(255,255,255,200);
        else                       led2.set(  0,  0,  0,200);
        if (preset.getSnap() == 2) led3.set(255,255,255,200);
        else                       led3.set(  0,  0,  0,200);
        initialize = TRUE;
      }
      else {
        if (lsw.pressed()) {
          if (preset.getSnap() == 0) jumpTo(MENU_MAIN);
          else {
            preset.setSnap(0);
            jumpTo(MENU_MAIN);
          }
        }
        if (csw.pressed()) {
          if (preset.getSnap() == 1) jumpTo(MENU_MAIN);
          else {
            preset.setSnap(1);
            jumpTo(MENU_MAIN);
          }
        }
        if (rsw.pressed()) {
          if (preset.getSnap() == 2) jumpTo(MENU_MAIN);
          else {
            preset.setSnap(2);
            jumpTo(MENU_MAIN);
          }
        }
        if (rsw.longPressed()) {
          jumpTo(MENU_MAIN);
        }
      }
      break;
  };
}
