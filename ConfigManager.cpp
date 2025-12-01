#include "ConfigManager.h"
#include "PedalboardUI.h"

ConfigManager configManager;

// UUIDs for the Service and Characteristics
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define COMMAND_CHAR_UUID   "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define DATA_CHAR_UUID      "beb5483e-36e1-4688-b7f5-ea07361b26a9"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("BLE Client Connected");
    };

    void onDisconnect(BLEServer* pServer) {
        Serial.println("BLE Client Disconnected");
        pServer->getAdvertising()->start();
    }
};

class CommandCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        uint8_t* data = pCharacteristic->getData();
        size_t len = pCharacteristic->getLength();
        configManager.handleBLECommand(data, len);
    }
};

class DataCallbacks: public BLECharacteristicCallbacks {
    void onRead(BLECharacteristic *pCharacteristic) {
        Serial.println("Data read requested");
        configManager.sendCurrentConfig();
    }
};

ConfigManager::ConfigManager() {
}

void ConfigManager::begin() {
    loadFromPreferences();
    setupBLE();
}

void ConfigManager::update() {
    // Nothing needed here anymore
}

void ConfigManager::handleBLECommand(uint8_t* data, size_t len) {
    if (len < 2) return;
    
    uint8_t cmd = data[0];
    uint8_t index = data[1];
    
    // CMD 1: Write Button Config
    if (cmd == 1 && index < 4 && len >= 7) {
        MidiButtonConfig newConfig;
        newConfig.type = data[2];
        newConfig.midiType = data[3];
        newConfig.value = data[4];
        newConfig.channel = data[5];
        newConfig.velocity = data[6];
        newConfig.enabled = 1;
        
        saveButtonConfig(index, newConfig);
        
        String msg = "Saved: B" + String(getCurrentBank() + 1) + " Btn" + String(index + 1);
        pedalboardUI.showStatusMessage(msg, CYAN);
        
        // Send updated config back
        sendCurrentConfig();
    }
    // CMD 2: Set Bank
    else if (cmd == 2) {
        uint8_t bank = index;
        setCurrentBank(bank);
        
        String msg = "Bank " + String(bank + 1);
        pedalboardUI.showStatusMessage(msg, MAGENTA);
        
        // Send new bank config back
        sendCurrentConfig();
    }
}

void ConfigManager::sendCurrentConfig() {
    if (!pDataCharacteristic) return;
    
    // 1 byte Bank + 4 buttons * 6 bytes = 25 bytes
    uint8_t response[25];
    response[0] = currentBank;
    
    for (int i = 0; i < 4; i++) {
        MidiButtonConfig cfg = configs[currentBank][i];
        int offset = 1 + (i * 6);
        response[offset + 0] = cfg.type;
        response[offset + 1] = cfg.midiType;
        response[offset + 2] = cfg.value;
        response[offset + 3] = cfg.channel;
        response[offset + 4] = cfg.velocity;
        response[offset + 5] = cfg.enabled;
    }
    
    pDataCharacteristic->setValue(response, 25);
    pDataCharacteristic->notify();
    
    Serial.println("Config sent via BLE");
}

void ConfigManager::setCurrentBank(uint8_t bank) {
    if (bank >= NUM_BANKS) return;
    currentBank = bank;
    Serial.printf("Switched to Bank %d\n", currentBank + 1);
    
    preferences.begin("midi-pedal", false);
    preferences.putUChar("bank", currentBank);
    preferences.end();
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
    
    currentBank = preferences.getUChar("bank", 0);
    if (currentBank >= NUM_BANKS) currentBank = 0;
    
    bool isInit = preferences.getBool("init_v3", false);
    preferences.end(); 
    
    if (!isInit) {
        Serial.println("First boot (v3) - loading defaults");
        loadDefaults(); 
    } else {
        Serial.println("Loading saved configs");
        preferences.begin("midi-pedal", false);
        for (int b = 0; b < NUM_BANKS; b++) {
            for (int i = 0; i < 4; i++) {
                String key = "b" + String(b) + "_btn" + String(i);
                if (preferences.isKey(key.c_str())) {
                    preferences.getBytes(key.c_str(), &configs[b][i], sizeof(MidiButtonConfig));
                } else {
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
            configs[b][i].velocity = 127;
            configs[b][i].enabled = 1;
            
            if (b == 0) configs[b][i].value = 60 + i;
            else if (b == 1) configs[b][i].value = 72 + i;
            else if (b == 2) configs[b][i].value = 48 + i;
            else configs[b][i].value = 36 + i;
            
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

    // Command characteristic (Write only)
    pCommandCharacteristic = pService->createCharacteristic(
        COMMAND_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pCommandCharacteristic->setCallbacks(new CommandCallbacks());

    // Data characteristic (Read + Notify)
    pDataCharacteristic = pService->createCharacteristic(
        DATA_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    pDataCharacteristic->setCallbacks(new DataCallbacks());
    pDataCharacteristic->addDescriptor(new BLE2902());

    // Set initial data
    sendCurrentConfig();

    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    
    Serial.println("BLE Config Service Started");
}
