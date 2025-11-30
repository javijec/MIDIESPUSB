#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Button types
enum ButtonType {
    BUTTON_MOMENTARY = 0,
    BUTTON_TOGGLE = 1
};

// MIDI Message types
enum MidiMessageType {
    MIDI_TYPE_NOTE = 0,
    MIDI_TYPE_CC = 1,
    MIDI_TYPE_PC = 2
};

struct MidiButtonConfig {
    uint8_t type;       // ButtonType
    uint8_t midiType;   // MidiMessageType
    uint8_t value;      // Note number or CC number
    uint8_t channel;    // MIDI Channel (1-16)
    uint8_t enabled;    // 1 = enabled, 0 = disabled
};

#define NUM_BANKS 4

class ConfigManager {
public:
    ConfigManager();
    
    void begin();
    void update(); // Call in loop for BLE housekeeping if needed
    
    // Bank Management
    void setCurrentBank(uint8_t bank);
    uint8_t getCurrentBank();
    void nextBank();
    bool deviceConnected = false;
    
    void loadFromPreferences();
    void setupBLE();
    
    // Default configuration
    void loadDefaults();
};

extern ConfigManager configManager;

#endif // CONFIG_MANAGER_H
