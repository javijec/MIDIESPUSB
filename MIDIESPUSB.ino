#include <Control_Surface.h>  // Librería para control MIDI
#include "ButtonManager.h"    // Incluye el gestor de botones
#include "ST7789_Graphics.h"
#include "PedalboardUI.h"     // Incluye la UI
#include "BankManager.h"      // Incluye el gestor de bancos
#include "ConfigManager.h"    // Incluye el gestor de configuración

// Variables para la pantalla
unsigned long lastUpdate = 0;

// La interfaz MIDI a utilizar (MIDI sobre USB, típico en ESP32 S2/S3)
USBMIDI_Interface midi;

// Instancia del gestor de botones
ButtonManager buttonManager;

const uint8_t velocity = 0x40;  // Velocidad de la nota (volumen máximo: 127/0x7F)

// --- Variables para el control de la nota (evita usar 'delay') ---
unsigned long noteOnTime = 0;
// La nota actualmente activa (usa la dirección de la nota que se está tocando)
MIDIAddress activeNoteAddress = { 0, Channel_1 };
// Duración de la nota en milisegundos. La nota se apagará automáticamente después de este tiempo.
const int NOTE_DURATION_MS = 200;

// Estado de los botones toggle (true = encendido, false = apagado)
bool toggleStates[4] = {false, false, false, false};

// Declaración de funciones
void handleButtonEvent(uint8_t id, uint8_t eventType);

void setup() {
  display.begin(50);
  display.enableTextAA(false);
  
  // Inicialización de bancos
  bankManager.begin();
  
  // Inicialización de la UI
  pedalboardUI.begin();
  pedalboardUI.updateBankLabel(bankManager.getCurrentBankName());
  
  // Inicialización de la interfaz MIDI
  midi.begin();
  
  // Inicialización de configuración (BLE + Preferencias)
  configManager.begin();
  
  // Inicialización de botones
  buttonManager.begin(handleButtonEvent);
}

void loop() {
  // Actualizar botones
  buttonManager.update();
  
  // Actualizar UI (animaciones, mensajes)
  pedalboardUI.update();

  // Apagado automático de la nota
  if (noteOnTime != 0 && millis() - noteOnTime >= NOTE_DURATION_MS) {
    midi.sendNoteOff(activeNoteAddress, velocity);
    noteOnTime = 0;
    activeNoteAddress = { 0, Channel_1 };
  }

  midi.update();
}

// Callback para eventos de botones
void handleButtonEvent(uint8_t id, uint8_t eventType) {
  // Ignore the idle state button (Index 0)
  if (id == 0) return;

  // Map physical buttons (1-4) to logical indices (0-3)
  uint8_t logicalId = id - 1;

  // Obtener configuración del botón
  MidiButtonConfig config = configManager.getButtonConfig(logicalId);

  // Lógica de cambio de banco: Long Press en Botón 4 (índice 3)
  if (logicalId == 3 && eventType == ButtonManager::EVENT_LONG_PRESSED) {
      bankManager.nextBank();
      pedalboardUI.updateBankLabel(bankManager.getCurrentBankName());
      pedalboardUI.showStatusMessage("Bank Changed", YELLOW);
      return; // No enviar nota si se usó para cambiar de banco
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
                      midi.sendNoteOn(noteToSend, velocity);
                      String msg = "Note ON " + String(config.value);
                      pedalboardUI.showStatusMessage(msg, GREEN);
                  } else {
                      // Apagar
                      midi.sendNoteOff(noteToSend, velocity);
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
      // Actualizar UI visualmente según el estado físico
      if (eventType == ButtonManager::EVENT_PRESSED) {
          pedalboardUI.setButtonState(logicalId, true, config.type);
          
          if (config.enabled) {
              if (config.midiType == MIDI_TYPE_NOTE) {
                  // Note On
                  MIDIAddress noteToSend = {config.value, (Channel)(config.channel)};
                  midi.sendNoteOn(noteToSend, velocity);
                  activeNoteAddress = noteToSend;
                  noteOnTime = millis();
                  
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
          pedalboardUI.setButtonState(logicalId, false, config.type);
          
          // En momentary mode, enviamos Note Off al soltar (solo para notas)
          if (config.enabled && config.midiType == MIDI_TYPE_NOTE) {
              MIDIAddress noteToSend = {config.value, (Channel)(config.channel)};
              midi.sendNoteOff(noteToSend, velocity);
              noteOnTime = 0; // Cancelar el auto-off
          }
      }
  }
}
