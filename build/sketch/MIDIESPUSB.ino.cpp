#include <Arduino.h>
#line 1 "C:\\Users\\javij\\OneDrive\\01-Programacion\\06-Arduino\\Pedalera MIDI\\MIDIESPUSB\\MIDIESPUSB.ino"
#include <Control_Surface.h>  // Librería para control MIDI
#include "Button_Driver.h"    // Incluye el driver del botón que creaste
#include "ST7789_Graphics.h"

// Variables para la pantalla
unsigned long lastUpdate = 0;
int counter = 0;
String lastCommand = "";

// Mensaje en pantalla por evento
unsigned long displayMessageTime = 0;
const unsigned long DISPLAY_MESSAGE_MS = 2000;
String displayMessage = "";

// La interfaz MIDI a utilizar (MIDI sobre USB, típico en ESP32 S2/S3)
USBMIDI_Interface midi;

// --- Configuración MIDI ---
// Nota MIDI para CLICK (Do Central o C4, Canal 1)
const MIDIAddress CLICK_ADDRESS{ MIDI_Notes::C[4], Channel_1 };
// Nota MIDI para DOUBLE CLICK (Mi 4, Canal 1)
const MIDIAddress DOUBLE_CLICK_ADDRESS{ MIDI_Notes::E[4], Channel_1 };
const uint8_t velocity = 0x40;  // Velocidad de la nota (volumen máximo: 127/0x7F)

// --- Variables para el control de la nota (evita usar 'delay') ---
unsigned long noteOnTime = 0;
// La nota actualmente activa (usa la dirección de la nota que se está tocando)
MIDIAddress activeNoteAddress = { 0, Channel_1 };
// Duración de la nota en milisegundos. La nota se apagará automáticamente después de este tiempo.
const int NOTE_DURATION_MS = 200;

void setup() {
  display.begin(50);
  display.enableTextAA(false);
  welcome();
  // Inicialización de la interfaz MIDI
  midi.begin();
}

void loop() {
  // 1. Mostrar mensaje en pantalla por eventos (handler registró displayMessage)
  if (displayMessageTime != 0) {
    // si aún no expiró
    if (millis() - displayMessageTime <= DISPLAY_MESSAGE_MS) {
      display.clearCenteredLine(80, 1, BLACK);
      display.drawCenteredText(80, displayMessage, GREEN, BLACK, 1);
    } else {
      // Expiró
      display.clearCenteredLine(80, 1, BLACK);
      displayMessageTime = 0;
      displayMessage = "";
    }
  }


  // 4. Apagado automático de la nota (para que no se quede sonando)
  // Verifica si la nota está activa y si ha pasado la duración definida.
  if (noteOnTime != 0 && millis() - noteOnTime >= NOTE_DURATION_MS) {

    // Enviar mensaje MIDI "Note Off" a la nota activa
    midi.sendNoteOff(activeNoteAddress, velocity);
    // Limpiamos la línea donde se muestran las notas (y=200)
    display.drawCenteredText(80, "                                                            ", GREEN, BLACK, 1);
    // Resetear el temporizador y la nota activa
    noteOnTime = 0;
    activeNoteAddress = { 0, Channel_1 };
  }



  // Esta función debe ser llamada siempre para manejar la comunicación MIDI
  midi.update();
}

void welcome() {
  display.clearScreen(BLACK);
  // Título principal
  display.drawCenteredText(20, "ESP32S3", CYAN, BLACK, 3);
  display.drawCenteredText(50, "MIDI Interface", YELLOW, BLACK, 1);
}

