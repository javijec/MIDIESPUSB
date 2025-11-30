#include "PedalboardUI.h"

PedalboardUI pedalboardUI;

PedalboardUI::PedalboardUI() {
}

void PedalboardUI::begin() {
    display.clearScreen(BLACK);
    
    // Header simple
    display.drawCenteredText(10, "MIDI CONTROLLER", CYAN, BLACK, 2);
    display.drawHLine(0, 30, display.getWidth(), DARKGRAY);
    
    // Bank label inicial
    updateBankLabel("Bank 1");
    
    // Dibujar los 4 botones en estado OFF inicial
    for (int i = 0; i < 4; i++) {
        drawToggleButton(i, false);
    }
}

void PedalboardUI::update() {
    // No hace nada - sin animaciones
}

void PedalboardUI::setButtonState(uint8_t index, bool state, uint8_t buttonType) {
    if (index >= 4) return;
    
    // Siempre usar drawToggleButton (simple ON/OFF)
    drawToggleButton(index, state);
}

void PedalboardUI::updateBankLabel(const String& bankName) {
    // Borra area del banco y redibuja
    display.fillRect(0, 40, display.getWidth(), 20, BLACK);
    display.drawCenteredText(45, bankName, YELLOW, BLACK, 2);
}

void PedalboardUI::showStatusMessage(const String& msg, uint16_t color) {
    // Barra de estado simple en el fondo
    int y = 150;
    display.fillRect(0, y - 10, display.getWidth(), 25, DARKGRAY);
    display.drawCenteredText(y, msg, color, DARKGRAY, 1);
}

void PedalboardUI::drawToggleButton(uint8_t index, bool state) {
    // Diseño simple rectangular
    const int BTN_WIDTH = 50;
    const int BTN_HEIGHT = 50;
    const int BTN_GAP = 10;
    const int BTN_Y = 100;
    
    // Calcular posición centrada
    int totalWidth = (4 * BTN_WIDTH) + (3 * BTN_GAP);
    int startX = (display.getWidth() - totalWidth) / 2;
    int x = startX + index * (BTN_WIDTH + BTN_GAP);
    
    // Colores directos
    uint16_t fillColor = state ? GREEN : DARKGRAY;
    uint16_t borderColor = state ? WHITE : GRAY;
    uint16_t textColor = state ? BLACK : WHITE;
    
    // Dibujar rectángulo
    display.fillRoundRect(x, BTN_Y, BTN_WIDTH, BTN_HEIGHT, 5, fillColor);
    display.drawRoundRect(x, BTN_Y, BTN_WIDTH, BTN_HEIGHT, 5, borderColor);
    
    // Texto ON/OFF
    String label = state ? "ON" : "OFF";
    int textX = x + (BTN_WIDTH - display.getTextWidth(label, 2)) / 2;
    int textY = BTN_Y + (BTN_HEIGHT - display.getCharHeight(2)) / 2;
    display.drawText(textX, textY, label, textColor, fillColor, 2);
    
    // Número debajo
    String numLabel = String(index + 1);
    int numY = BTN_Y + BTN_HEIGHT + 10;
    display.drawCenteredText(numY, numLabel, WHITE, BLACK, 1);
}
