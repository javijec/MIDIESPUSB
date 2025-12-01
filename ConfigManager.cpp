#include "ConfigManager.h"
#include "PedalboardUI.h"

ConfigManager configManager;

// UUIDs for the Service and Characteristic
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        // deviceConnected = true; 
    };

    void onDisconnect(BLEServer* pServer) {
        // deviceConnected = false;
        // Restart advertising
        pServer->getAdvertising()->start();
    }
};

class ConfigCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        // Use raw data access to avoid String truncation with null bytes
        uint8_t* data = pCharacteristic->getData();
        size_t len = pCharacteristic->getLength();
        
        if (len >= 7) {
            // Format: [CMD, INDEX, TYPE, MIDITYPE, VALUE, CHANNEL, VELOCITY]
            uint8_t cmd = data[0];
            uint8_t index = data[1];
            
            // CMD 1: Write Button Config
            if (cmd == 1 && index < 4) {
                MidiButtonConfig newConfig;
                newConfig.type = data[2];
                newConfig.midiType = data[3];
                newConfig.value = data[4];
                newConfig.channel = data[5];
                newConfig.velocity = data[6];
                newConfig.enabled = 1;
                
                configManager.saveButtonConfig(index, newConfig);
                
                // Feedback
                String msg = "Saved: B" + String(configManager.getCurrentBank() + 1) + " Btn" + String(index + 1);
                pedalboardUI.showStatusMessage(msg, CYAN);
            }
            // CMD 2: Set Bank
            else if (cmd == 2) {
                uint8_t bank = index; // Reuse index byte for bank number
                configManager.setCurrentBank(bank);
                
                String msg = "Bank " + String(bank + 1);
                pedalboardUI.showStatusMessage(msg, MAGENTA);
            }
        }
    }
    
    void onRead(BLECharacteristic *pCharacteristic) {
        // Value is already set by updateBLEValue()
        Serial.println("Read request received");
    }
};

ConfigManager::ConfigManager() {
}

void ConfigManager::begin() {
    loadFromPreferences();
    setupBLE();
}

void ConfigManager::update() {
    // BLE housekeeping if needed
}

void ConfigManager::setCurrentBank(uint8_t bank) {
    if (bank >= NUM_BANKS) return;
    currentBank = bank;
    Serial.printf("Switched to Bank %d\n", currentBank + 1);
    
    // Optional: Save current bank to preferences so it persists?
    preferences.begin("midi-pedal", false);
    preferences.putUChar("bank", currentBank);
    preferences.end();
    
    updateBLEValue();
}

uint8_t ConfigManager::getCurrentBank() {
    return currentBank;
}

void ConfigManager::nextBank() {
    uint8_t next = (currentBank + 1) % NUM_BANKS;
    setCurrentBank(next);
}

void ConfigManager::prevBank() {
    uint8_t prev = (currentBank - 1 + NUM_BANKS) % NUM_BANKS;
    setCurrentBank(prev);
}

void ConfigManager::loadFromPreferences() {
    preferences.begin("midi-pedal", false);
    
    Serial.println("Loading preferences...");
    
    // Load last used bank
    currentBank = preferences.getUChar("bank", 0);
    if (currentBank >= NUM_BANKS) currentBank = 0;
    
    // Check if initialized
    bool isInit = preferences.getBool("init_v3", false); // New init flag for v3 (velocity)
    preferences.end(); 
    
    if (!isInit) {
        Serial.println("First boot (v3) - loading defaults");
        loadDefaults(); 
    } else {
        Serial.println("Loading saved configs");
        preferences.begin("midi-pedal", false);
        // Load configs for all banks
        for (int b = 0; b < NUM_BANKS; b++) {
            for (int i = 0; i < 4; i++) {
                String key = "b" + String(b) + "_btn" + String(i);
                if (preferences.isKey(key.c_str())) {
                    preferences.getBytes(key.c_str(), &configs[b][i], sizeof(MidiButtonConfig));
                } else {
                    // Fallback
                    configs[b][i] = {BUTTON_MOMENTARY, MIDI_TYPE_NOTE, (uint8_t)(60 + i), 1, 127, 1};
                }
            }
        }
        preferences.end();
    }
}

void ConfigManager::loadDefaults() {
    preferences.begin("midi-pedal", false);
    
    for (int b = 0; b < NUM_BANKS; b++) {
        for (int i = 0; i < 4; i++) {
            configs[b][i].type = BUTTON_MOMENTARY;
            configs[b][i].midiType = MIDI_TYPE_NOTE;
            configs[b][i].channel = 1;
            configs[b][i].velocity = 127; // Default max velocity
            configs[b][i].enabled = 1;
            
            // Different defaults per bank
            if (b == 0) configs[b][i].value = 60 + i;      // Bank 1: C4...
            else if (b == 1) configs[b][i].value = 72 + i; // Bank 2: C5...
            else if (b == 2) configs[b][i].value = 48 + i; // Bank 3: C3...
            else configs[b][i].value = 36 + i;             // Bank 4: C2... (Drums?)
            
            String key = "b" + String(b) + "_btn" + String(i);
            preferences.putBytes(key.c_str(), &configs[b][i], sizeof(MidiButtonConfig));
        }
    }
    
    preferences.putBool("init_v3", true);
    preferences.putUChar("bank", 0);
    currentBank = 0;
    
    Serial.println("Defaults loaded (v3)");
    preferences.end();
}

void ConfigManager::saveButtonConfig(uint8_t index, MidiButtonConfig config) {
    if (index >= 4) return;
    
    configs[currentBank][index] = config;
    
    preferences.begin("midi-pedal", false);
    String key = "b" + String(currentBank) + "_btn" + String(index);
    preferences.putBytes(key.c_str(), &config, sizeof(MidiButtonConfig));
    preferences.end();
    
    Serial.printf("Saved Bank%d Btn%d\n", currentBank, index);
    
    updateBLEValue();
}

MidiButtonConfig ConfigManager::getButtonConfig(uint8_t index) {
    if (index >= 4) {
        return {BUTTON_MOMENTARY, MIDI_TYPE_NOTE, 0, 1, 127, 0};
    }
    return configs[currentBank][index];
}

void ConfigManager::setupBLE() {
    BLEDevice::init("MIDI Pedalboard Config");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pConfigCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

    pConfigCharacteristic->setCallbacks(new ConfigCallbacks());
    
    // Set initial value immediately
    updateBLEValue();

    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Config Service Started");
}

void ConfigManager::updateBLEValue() {
    if (!pConfigCharacteristic) return;
    
    // 1 byte Bank + 4 buttons * 6 bytes = 25 bytes
    uint8_t response[25];
    response[0] = currentBank;
    
    for (int i = 0; i < 4; i++) {
        // Use internal array directly or getter
        MidiButtonConfig cfg = configs[currentBank][i];
        int offset = 1 + (i * 6);
        response[offset + 0] = cfg.type;
        response[offset + 1] = cfg.midiType;
        response[offset + 2] = cfg.value;
        response[offset + 3] = cfg.channel;
        response[offset + 4] = cfg.velocity;
        response[offset + 5] = cfg.enabled;
    }
    
    pConfigCharacteristic->setValue(response, 25);
    // Notify if we want to push updates? For now just set value for next read.
}
