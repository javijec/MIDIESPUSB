#ifndef MIDI_PEDALBOARD_H
#define MIDI_PEDALBOARD_H

#include <Control_Surface.h>
#include "ButtonManager.h"
#include "ConfigManager.h"
#include "MenuManager.h"
#include "PedalboardUI.h"

class MidiPedalboard {
public:
    MidiPedalboard();
    void begin();
    void update();

    // Static callback wrapper for ButtonManager
    static void handleButtonEventWrapper(uint8_t id, uint8_t eventType);

private:
    void handleButtonEvent(uint8_t id, uint8_t eventType);

    // MIDI Interface
    USBMIDI_Interface midi;

    // Button Manager
    ButtonManager buttonManager;

    // State variables
    static const uint8_t velocity = 0x40;
    bool toggleStates[4];
    
    // Singleton instance pointer for the static callback
    static MidiPedalboard* instance;
};

extern MidiPedalboard pedalboard;

#endif // MIDI_PEDALBOARD_H
