#ifndef PRESET_H
#define PRESET_H

#define MIDI_CC        0xB0
#define MIDI_PC        0xC0
#define MIDI_NOTE_OFF  0x80
#define MIDI_NOTE_ON   0x90
#define MIDI_NOTE_SNAP   69

#define MIDI_NOTE_FS4    52
#define MIDI_NOTE_FS5    53
#define MIDI_NOTE_FS6    48

#define FS_MODE          71
#define FS_MODE_SCROLL    1
#define FS_MODE_STOMP     0

void sendMidi(uint8_t command, uint8_t note, uint8_t velocity) {
  //Serial.write("|SENDMIDI|");
  Serial.write(command);
  Serial.write(note);
  Serial.write(velocity);
}

struct FXStomp {
  uint8_t note = 0;
  bool    isOn = false;

  FXStomp(uint8_t note) {
    this->note = note;
  }

  void send() {
    //Serial.write("|FXSTOMP|");
    Serial.write(MIDI_CC);
    Serial.write(note);
    Serial.write(isOn ? 127 : 0);
  }

  void toggle() {
    if (isOn) set(false);
    else      set(true);
    send();
  }

  void set(bool state) { isOn = state; }
  bool get()           { return isOn;  }
};

struct Snapshot {
  FXStomp fs4 = FXStomp(MIDI_NOTE_FS4);
  FXStomp fs5 = FXStomp(MIDI_NOTE_FS5);
  FXStomp fs6 = FXStomp(MIDI_NOTE_FS6);

  void reset() {
    fs4.set(false);
    fs5.set(false);
    fs6.set(false);
  }
};

struct Preset {
  uint8_t  presetNum = 0, snapshotNum = 0;
  Snapshot snapshots[3];

  void sendPreset() {
    //Serial.write("|SENDPRESET|");
    Serial.write(MIDI_PC);
    Serial.write(presetNum);
    resetSnapshots();
  }
  void setPreset(uint8_t newPreset) {
    presetNum = newPreset;
    sendPreset();
  }
  void nextPreset() {
    if (presetNum < 125) presetNum += 1;
    else                 presetNum  = 0;
    sendPreset();
  }
  void prevPreset() {
    if (presetNum > 0) presetNum -= 1;
    else               presetNum  = 125;
    sendPreset();
  }

  void sendSnapshot() {
    //Serial.write("|SENDSNAPSHOT|");
    Serial.write(MIDI_CC);
    Serial.write(MIDI_NOTE_SNAP);
    Serial.write(snapshotNum);
  }
  void setSnapshot(uint8_t newSnapshot) {
    snapshotNum = newSnapshot;
    sendSnapshot();
  }
  Snapshot* getSnapshot() {
    return &snapshots[snapshotNum];
  }
  void resetSnapshots() {
    for (int i=0; i<3; i++) {
      snapshots[i].reset();
    }
    snapshotNum = 0;
  }

  void send() {
    sendPreset();
    sendSnapshot();
  }
};

#endif
