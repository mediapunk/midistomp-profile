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
          led1.set(0,  0,  0,  1000);
          led2.set(0,  0,  0,  1000);
          led3.set(50, 50, 50, 1000);

          intro_t = timer;
          jumpTo(MENU_MAIN);
        }
      }
      break;

    case MENU_MAIN:
      if (!initialize && timer >= intro_t+1000) {
        led1.set(snap_p->fs4.get() ? 255 : 0,  0,  0,  200);
        led2.set(snap_p->fs5.get() ? 255 : 0,  0,  0,  200);
        led3.set(                          50, 50, 50, 200);
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
        if (rsw.isPressed) jumpTo(MENU_SUB);

        if (lsw.isLongPressed) {
          // Talkback on/off
          sendMidi(MIDI_NOTE_ON, 0, 127);
          led1.set(255,255,255,0);
          initialize = false;
        }
        if (csw.isLongPressed) {
          // Record On
          sendMidi(MIDI_NOTE_ON, 1, 127);
          if (snap_p->fs5.get()) {
            led2.blink(2, 50);
          }
          else {
            led2.set(255,0,0,0);
            initialize = false;
          }
        }
        if (rsw.isLongPressed) {
          // Stop All
          sendMidi(MIDI_NOTE_ON, 2, 127);
          led3.blink(2, 50);
        }
      }
      break;

    case MENU_SUB:
      if (!initialize) {
        led1.set(50,  50,  50, 200);
        led3.set(255, 135, 0,  200);
        if (preset.presetNum > 60) {
          led2.set(50,0,0, 200);
        }
        else {
          led2.set(255,20,0, 200);
        }
        initialize = true;
      }
      else {
        if (lsw.isPressed) jumpTo(MENU_SUB_2);
        if (preset.presetNum <= 60) {
          if (csw.isPressed) {
            // Start/Stop Metronome
            sendMidi(MIDI_NOTE_ON, (preset.presetNum*2)+4, 127);
            /*doesnt show up anyway because of the jumpTo*/
            /*led2.blink(2, 50);*/
            jumpTo(MENU_MAIN);
          }
        }
        if (rsw.isPressed) jumpTo(MENU_CHANGE_PRESET);

        /*if (lsw.isLongPressed) {}*/
        /*if (csw.isLongPressed) {}*/
        if (rsw.isLongPressed) jumpTo(MENU_MAIN);
      }
      break;

    case MENU_SUB_2:
      if (!initialize) {
        if (preset.presetNum > 60) {
          led1.set(50,0,0, 200);
        }
        else {
          led1.set(255,20,0, 200);
        }
        led2.set(255, 0, 0,   200);
        led3.set(255, 0, 255, 200);
        initialize = true;
      }
      else {
        if (preset.presetNum <= 60) {
          if (lsw.isPressed) {
            // Start/Stop Back Tracks
            sendMidi(MIDI_NOTE_ON, (preset.presetNum*2)+5, 127);
            /*led1.blink(2, 50);*/
            jumpTo(MENU_MAIN);
          }
        }
        if (csw.isPressed) {
          // Record Stop
          sendMidi(MIDI_NOTE_ON, 3, 127);
          /*led2.blink(2, 50);*/
          jumpTo(MENU_MAIN);
        }
        if (rsw.isPressed) jumpTo(MENU_RESET);

        /*if (lsw.isLongPressed) {}*/
        /*if (csw.isLongPressed) {}*/
        if (rsw.isLongPressed) jumpTo(MENU_MAIN);
      }
      break;

    case MENU_RESET:
      if (!initialize) {
        led1.set(255, 0,255, 200);
        led2.set(255,20,  0, 200);
        led3.set(  0, 0,  0, 200);
        initialize = true;
      } else {
        if (lsw.isPressed) {
          jumpTo(MENU_INTRO);
        }
        if (csw.isPressed) {
          jumpTo(MENU_MIDI_ONLY);
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
          led1.blink(2, 100);
          led3.blink(2, 100);
        }
        if (rsw.isLongPressed) {
          preset.setPreset(preset.presetNum < 105 ? preset.presetNum+20 : 125);
          led3.blink(2, 100);
        }
      }
      break;

    case MENU_MIDI_ONLY:
      if (!initialize) {
        led1.set(255,20,0, 200);
        led2.set(255,20,0, 200);
        led3.set(255,20,0, 200);
        initialize = true;
      }
      else {
        if (lsw.isPressed) {
          sendMidi(MIDI_CC, 1, 127);
          sendMidi(MIDI_CC, 1, 0);
          led1.blink(2, 50);
        }
        if (csw.isPressed) {
          sendMidi(MIDI_CC, 2, 127);
          sendMidi(MIDI_CC, 2, 0);
          led2.blink(2, 50);
        }
        if (rsw.isPressed) {
          sendMidi(MIDI_CC, 3, 127);
          sendMidi(MIDI_CC, 3, 0);
          led3.blink(2, 50);
        }

        if (lsw.isLongPressed) {
          sendMidi(MIDI_CC, 4, 127);
          sendMidi(MIDI_CC, 4, 0);
          led1.blink(2, 100);
        }
        if (csw.isLongPressed) {
          sendMidi(MIDI_CC, 5, 127);
          sendMidi(MIDI_CC, 5, 0);
          led2.blink(2, 100);
        }
        if (rsw.isLongPressed) {
          jumpTo(MENU_RESET);
        }
      }
      break;

    default:
      jumpTo(MENU_INTRO);
      break;
  };
}
