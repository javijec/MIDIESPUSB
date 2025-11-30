#include "ConfigManager.h"

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
        String value = pCharacteristic->getValue();
        
        if (value.length() >= 6) {
            // Format: [CMD, INDEX, TYPE, MIDITYPE, VALUE, CHANNEL]
            // CMD: 1 = Write
            uint8_t cmd = value[0];
            uint8_t index = value[1];
            
            if (cmd == 1 && index < 4) {
                MidiButtonConfig newConfig;
                newConfig.type = value[2];
                newConfig.midiType = value[3];
                newConfig.value = value[4];
                newConfig.channel = value[5];
                newConfig.enabled = 1;
                
                configManager.saveButtonConfig(index, newConfig);
                Serial.printf("Updated Config for Button %d\n", index);
            }
        }
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

void ConfigManager::loadFromPreferences() {
    preferences.begin("midi-pedal", false);
    
    // Check if initialized
    if (!preferences.getBool("init", false)) {
        loadDefaults();
        preferences.putBool("init", true);
    } else {
        // Load configs
        for (int i = 0; i < 4; i++) {
            String key = "btn" + String(i);
            if (preferences.isKey(key.c_str())) {
                preferences.getBytes(key.c_str(), &configs[i], sizeof(MidiButtonConfig));
            } else {
                // Fallback if key missing
                configs[i] = {BUTTON_MOMENTARY, MIDI_TYPE_NOTE, (uint8_t)(60 + i), 1, 1};
            }
        }
    }
    preferences.end();
}

void ConfigManager::loadDefaults() {
    // Default: Buttons 1-4 send Notes C4, D4, E4, F4 on Channel 1
    for (int i = 0; i < 4; i++) {
        configs[i].type = BUTTON_MOMENTARY;
        configs[i].midiType = MIDI_TYPE_NOTE;
        configs[i].value = 60 + i; // C4, D4...
        configs[i].channel = 1;
        configs[i].enabled = 1;
        
        saveButtonConfig(i, configs[i]);
    }
}

void ConfigManager::saveButtonConfig(uint8_t index, MidiButtonConfig config) {
    if (index >= 4) return;
    
    configs[index] = config;
    
    preferences.begin("midi-pedal", false);
    String key = "btn" + String(index);
    preferences.putBytes(key.c_str(), &config, sizeof(MidiButtonConfig));
    preferences.end();
}

MidiButtonConfig ConfigManager::getButtonConfig(uint8_t index) {
    if (index >= 4) {
        return {BUTTON_MOMENTARY, MIDI_TYPE_NOTE, 0, 1, 0};
    }
    return configs[index];
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
    
    // Set initial value (maybe just a handshake or version)
    pConfigCharacteristic->setValue("READY");

    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Config Service Started");
}
