#include "MidiPedalboard.h"

MidiPedalboard* MidiPedalboard::instance = nullptr;

MidiPedalboard::MidiPedalboard() {
    instance = this;
    for (int i = 0; i < 4; i++) {
        toggleStates[i] = false;
        activeNotes[i] = {0, 1, 0, false};
    }
}

void MidiPedalboard::handleButtonEventWrapper(uint8_t id, uint8_t eventType) {
    if (instance) {
        instance->handleButtonEvent(id, eventType);
    }
}

void MidiPedalboard::begin() {
    Serial.begin(115200);
    // delay(500); // Wait for serial - removed to speed up boot or moved if needed
    Serial.println("Starting MIDI Pedalboard...");
    
    display.begin(50);
    display.enableTextAA(false);
    
    // *** CRITICAL: Load configuration first ***
    configManager.begin();
    Serial.println("Config loaded");
    
    // Inicialización de la UI
    pedalboardUI.begin();
    pedalboardUI.updateBankLabel("Bank " + String(configManager.getCurrentBank() + 1));
    
    // Inicialización del Menú
    menuManager.begin();
    
    // Redibujar botones según configuración guardada
    for (int i = 0; i < 4; i++) {
        MidiButtonConfig cfg = configManager.getButtonConfig(i);
        Serial.printf("Button %d: Type=%d, MidiType=%d, Value=%d\n", i, cfg.type, cfg.midiType, cfg.value);
        pedalboardUI.setButtonState(i, false, cfg.type); // Draw with correct type
    }
    
    // Inicialización de la interfaz MIDI
    midi.begin();
    
    // Inicialización de botones
    buttonManager.begin(handleButtonEventWrapper);
    
    Serial.println("Setup complete!");
}

void MidiPedalboard::update() {
    // Actualizar botones
    buttonManager.update();

    // BLE housekeeping
    configManager.update(); 

    midi.update();
}

void MidiPedalboard::handleButtonEvent(uint8_t id, uint8_t eventType) {
    // Ignore the idle state button (Index 0)
    if (id == 0) return;

    // Map physical buttons (1-4) to logical indices (0-3)
    // Inverted order: 1->3, 2->2, 3->1, 4->0
    uint8_t logicalId = 3 - (id - 1);

    // *** MENU HANDLING ***
    if (menuManager.isActive()) {
        menuManager.handleButton(logicalId, eventType);
        return; // Don't process MIDI or other logic if menu is active
    }
    
    // Obtener configuración del botón
    MidiButtonConfig config = configManager.getButtonConfig(logicalId);

    // Bank Switching Logic
    // Button 1 (Index 0) Long Press: Previous Bank
    if (logicalId == 0 && eventType == ButtonManager::EVENT_LONG_PRESSED) {
        // Turn off any active toggles from previous bank before switching?
        // For now, just switch.
        configManager.prevBank();
        
        // Visual Feedback
        String bankName = "Bank " + String(configManager.getCurrentBank() + 1);
        pedalboardUI.updateBankLabel(bankName);
        pedalboardUI.showStatusMessage(bankName, MAGENTA);
        
        // Redraw buttons
        for (int i = 0; i < 4; i++) {
            MidiButtonConfig cfg = configManager.getButtonConfig(i);
            toggleStates[i] = false; 
            pedalboardUI.setButtonState(i, false, cfg.type);
        }
        return;
    }

    // Button 4 (Index 3) Long Press: Next Bank
    if (logicalId == 3 && eventType == ButtonManager::EVENT_LONG_PRESSED) {
        configManager.nextBank();
        
        // Visual Feedback
        String bankName = "Bank " + String(configManager.getCurrentBank() + 1);
        pedalboardUI.updateBankLabel(bankName);
        pedalboardUI.showStatusMessage(bankName, MAGENTA);
        
        // Redraw buttons
        for (int i = 0; i < 4; i++) {
            MidiButtonConfig cfg = configManager.getButtonConfig(i);
            toggleStates[i] = false; 
            pedalboardUI.setButtonState(i, false, cfg.type);
        }
        return; 
    }

    // --- TOGGLE MODE ---
    if (config.type == BUTTON_TOGGLE) {
        // Solo actuar en el evento de click (pressed)
        if (eventType == ButtonManager::EVENT_PRESSED) {
            // Cambiar estado
            toggleStates[logicalId] = !toggleStates[logicalId];
            
            // Actualizar UI
            pedalboardUI.setButtonState(logicalId, toggleStates[logicalId], config.type);
            
            if (config.enabled) {
                if (config.midiType == MIDI_TYPE_NOTE) {
                    MIDIAddress noteToSend = {config.value, (Channel)(config.channel)};
                    if (toggleStates[logicalId]) {
                        // Encender
                        midi.sendNoteOn(noteToSend, config.velocity);
                        String msg = "Note ON " + String(config.value);
                        pedalboardUI.showStatusMessage(msg, GREEN);
                    } else {
                        // Apagar
                        midi.sendNoteOff(noteToSend, config.velocity);
                        String msg = "Note OFF " + String(config.value);
                        pedalboardUI.showStatusMessage(msg, RED);
                    }
                }
                else if (config.midiType == MIDI_TYPE_CC) {
                    MIDIAddress ccToSend = {config.value, (Channel)(config.channel)};
                    uint8_t ccValue = toggleStates[logicalId] ? 127 : 0;
                    midi.sendControlChange(ccToSend, ccValue);
                    String msg = "CC " + String(config.value) + ": " + String(ccValue);
                    pedalboardUI.showStatusMessage(msg);
                }
                else if (config.midiType == MIDI_TYPE_PC) {
                    // PC no tiene mucho sentido en toggle, pero lo enviaremos solo al encender
                    if (toggleStates[logicalId]) {
                        midi.sendProgramChange((Channel)(config.channel), config.value);
                        String msg = "PC " + String(config.value);
                        pedalboardUI.showStatusMessage(msg);
                    }
                }
            }
        }
    }
    // --- MOMENTARY MODE ---
    else {
        if (eventType == ButtonManager::EVENT_PRESSED) {
            // Visual ON
            pedalboardUI.setButtonState(logicalId, true, config.type);
            
            if (config.enabled) {
                // Track this note/CC
                activeNotes[logicalId].value = config.value;
                activeNotes[logicalId].channel = config.channel;
                activeNotes[logicalId].midiType = config.midiType;
                activeNotes[logicalId].active = true;

                if (config.midiType == MIDI_TYPE_NOTE) {
                    // Note On
                    MIDIAddress noteToSend = {config.value, (Channel)(config.channel)};
                    midi.sendNoteOn(noteToSend, config.velocity);
                    
                    String msg = "Note " + String(config.value);
                    pedalboardUI.showStatusMessage(msg);
                } 
                else if (config.midiType == MIDI_TYPE_CC) {
                    // Control Change
                    MIDIAddress ccToSend = {config.value, (Channel)(config.channel)};
                    midi.sendControlChange(ccToSend, 127); // Send max value
                    
                    String msg = "CC " + String(config.value);
                    pedalboardUI.showStatusMessage(msg);
                }
                else if (config.midiType == MIDI_TYPE_PC) {
                    // Program Change
                    midi.sendProgramChange((Channel)(config.channel), config.value);
                    
                    String msg = "PC " + String(config.value);
                    pedalboardUI.showStatusMessage(msg);
                }
            }
        } 
        else if (eventType == ButtonManager::EVENT_RELEASED) {
            // Visual OFF
            pedalboardUI.setButtonState(logicalId, false, config.type);
            
            // Check if we have an active note for this button
            if (activeNotes[logicalId].active) {
                uint8_t val = activeNotes[logicalId].value;
                uint8_t ch = activeNotes[logicalId].channel;
                uint8_t type = activeNotes[logicalId].midiType;
                
                if (type == MIDI_TYPE_NOTE) {
                    MIDIAddress noteToSend = {val, (Channel)(ch)};
                    // Use standard velocity for Note Off or stored? 
                    // Standard practice is 0 or same velocity. Let's use 0x40 or 0.
                    // Actually, Note Off velocity is often ignored or used for release velocity.
                    // Let's use a default 64 for release to be safe, or we could track velocity too.
                    // For simplicity, let's use 64 (0x40) for Note Off.
                    midi.sendNoteOff(noteToSend, 0x40);
                }
                
                activeNotes[logicalId].active = false;
            }
        }
    }
}

// Global instance
MidiPedalboard pedalboard;
