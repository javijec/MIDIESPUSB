#include <Arduino.h>
#line 1 "C:\\Users\\javij\\OneDrive\\01-Programacion\\06-Arduino\\Pedalera MIDI\\MIDIESPUSB\\MIDIESPUSB.ino"
#include "MidiPedalboard.h"

void setup() {
  pedalboard.begin();
}

void loop() {
  pedalboard.update();
}

