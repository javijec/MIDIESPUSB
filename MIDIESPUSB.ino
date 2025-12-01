#include "MidiPedalboard.h"

void setup() {
  pedalboard.begin();
}

void loop() {
  pedalboard.update();
}
