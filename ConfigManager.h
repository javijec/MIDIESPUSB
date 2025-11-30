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
    
    // Default configuration
    void loadDefaults();

    // Get configuration for a specific button (0-3) in the current bank
    MidiButtonConfig getButtonConfig(uint8_t index);
    
    // Save configuration for current bank
    void saveButtonConfig(uint8_t index, MidiButtonConfig config);

    void updateBLEValue(); // Helper to update char value

private:
    Preferences preferences;
    MidiButtonConfig configs[NUM_BANKS][4];
    uint8_t currentBank = 0;
    
    // BLE
    BLEServer* pServer = nullptr;
    BLECharacteristic* pConfigCharacteristic = nullptr;
    
    void loadFromPreferences();
    void setupBLE();
};

extern ConfigManager configManager;

#endif // CONFIG_MANAGER_H
