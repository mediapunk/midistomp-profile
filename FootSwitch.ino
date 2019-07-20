#include "FootSwitch.h"
#include "LED_RGB.h"
#include "Switch.h"
#include "Preset.h"

uint8_t  event;
long     timer, intro_t;
bool     initialize;

LED_RGB  led1(LED_LEFT);
LED_RGB  led2(LED_CENTER);
LED_RGB  led3(LED_RIGHT);

Switch   lsw(PIN_LEFT);
Switch   csw(PIN_CENTER);
Switch   rsw(PIN_RIGHT);

Preset   preset;
Snapshot *snap_p;

void jumpTo(uint8_t newEvent) {
  event      = newEvent;
  initialize = false;
}

void setup() {
  Serial.begin(31250);
  /*Serial.begin(115200);*/
  LED_RGB::init(PIN_LATCH, PIN_CLOCK, PIN_DATA);
  event      = 0;
  initialize = false;
  timer      = 0;
  intro_t    = 0;

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
  // Sets current time for loop
  timer = millis();

  LED_RGB::loop();
  led1.run();
  led2.run();
  led3.run();

  lsw.loop(timer);
  csw.loop(timer);
  rsw.loop(timer);

  snap_p = preset.getSnapshot();

  switch(event) {
    case MENU_INTRO:
      if (!initialize) {
        led1.showoff(2);
        led2.showoff(1);
        led3.showoff(0);
        initialize = true;
      } else {
        if (lsw.isLongPressed || csw.isLongPressed || rsw.isLongPressed) {
          sendMidi(MIDI_CC, FS_MODE, FS_MODE_STOMP);

          preset.setPreset(1);
          preset.setPreset(0);

          led1.set(0,255,0, 1000);
          led2.set(0,255,0, 1000);
          led3.set(0,255,0, 1000);

          jumpTo(MENU_OUTRO);
        }
      }
      break;

    case MENU_OUTRO:
      if (!initialize) {
        intro_t    = timer;
        initialize = true;
      } else {
        if (timer >= intro_t+1300) {
          led1.set(0,0,0, 1000);
          led2.set(0,0,0, 1000);
          led3.set(0,0,0, 1000);

          intro_t = timer;
          jumpTo(MENU_MAIN);
        }
      }
      break;

    case MENU_MAIN:
      if (!initialize && timer >= intro_t+1000) {
        led1.set(snap_p->fs4.get() ? 255 : 0,0,0, 200);
        led2.set(snap_p->fs5.get() ? 255 : 0,0,0, 200);
        led3.set(snap_p->fs6.get() ? 255 : 0,0,0, 200);
        initialize = true;
      } else {
        if (lsw.isPressed) {
          snap_p->fs4.toggle();
          led1.set(snap_p->fs4.get() ? 255 : 0,0,0, 50);
        }
        if (csw.isPressed) {
          snap_p->fs5.toggle();
          led2.set(snap_p->fs5.get() ? 255 : 0,0,0, 50);
        }
        if (rsw.isPressed) {
          snap_p->fs6.toggle();
          led3.set(snap_p->fs6.get() ? 255 : 0,0,0, 50);
        }

        if (lsw.isLongPressed) jumpTo(MENU_HIPBOX);
        if (csw.isLongPressed) jumpTo(MENU_CHANGE_SNAP);
        if (rsw.isLongPressed) jumpTo(MENU_CHANGE_PRESET);
      }
      break;

    case MENU_HIPBOX:
      if (!initialize) {
        if (preset.presetNum > 41) {
          led1.set(50,0,0, 200);
          led2.set(50,0,0, 200);
          led3.set(50,0,0, 200);
        }
        else {
          led1.set(255,20,0, 200);
          led2.set(255,20,0, 200);
          led3.set(255,20,0, 200);
        }
        initialize = true;
      }
      else {
        if (preset.presetNum <= 41) {
          if (lsw.isPressed) {
            sendMidi(MIDI_NOTE_ON, (preset.presetNum*3), 127);
            led1.blink(2, 50);
          }
          if (csw.isPressed) {
            sendMidi(MIDI_NOTE_ON, ((preset.presetNum*3)+1), 127);
            led2.blink(2, 50);
          }
          if (rsw.isPressed) {
            sendMidi(MIDI_NOTE_ON, ((preset.presetNum*3)+2), 127);
            led3.blink(2, 50);
          }
        }
        if (lsw.isLongPressed)                      jumpTo(MENU_INTRO);
        if (csw.isLongPressed || rsw.isLongPressed) jumpTo(MENU_MAIN);
      }
      break;

    case MENU_CHANGE_SNAP:
      if (!initialize) {
        if (preset.snapshotNum == 0) led1.set(255,255,255,200);
        else                         led1.set(  0,  0,  0,200);
        if (preset.snapshotNum == 1) led2.set(255,255,255,200);
        else                         led2.set(  0,  0,  0,200);
        if (preset.snapshotNum == 2) led3.set(255,255,255,200);
        else                         led3.set(  0,  0,  0,200);
        initialize = true;
      }
      else {
        if (lsw.isPressed) {
          if (preset.snapshotNum == 0) jumpTo(MENU_MAIN);
          else {
            preset.setSnapshot(0);
            jumpTo(MENU_MAIN);
          }
        }
        if (csw.isPressed) {
          if (preset.snapshotNum == 1) jumpTo(MENU_MAIN);
          else {
            preset.setSnapshot(1);
            jumpTo(MENU_MAIN);
          }
        }
        if (rsw.isPressed) {
          if (preset.snapshotNum == 2) jumpTo(MENU_MAIN);
          else {
            preset.setSnapshot(2);
            jumpTo(MENU_MAIN);
          }
        }
        if (rsw.isLongPressed) {
          jumpTo(MENU_MAIN);
        }
      }
      break;

    case MENU_CHANGE_PRESET:
      if (!initialize) {
        led1.set(255,135,0, 200);
        led2.set(  0,  0,0, 200);
        led3.set(255,135,0, 200);
        initialize = true;
      }
      else {
        if (lsw.isPressed) {
          preset.prevPreset();
          led1.blink(2, 50);
        }
        if (csw.isPressed) {
          jumpTo(MENU_MAIN);
        }
        if (rsw.isPressed) {
          preset.nextPreset();
          led3.blink(2, 50);
        }

        if (lsw.isLongPressed) {
          preset.setPreset(preset.presetNum > 20 ? preset.presetNum-20 : 0);
          led1.blink(2, 100);
        }
        if (csw.isLongPressed) {
          preset.setPreset(0);
        }
        if (rsw.isLongPressed) {
          preset.setPreset(preset.presetNum < 105 ? preset.presetNum+20 : 125);
          led3.blink(2, 100);
        }
      }
      break;

    default:
      jumpTo(MENU_INTRO);
      break;
  };
}
