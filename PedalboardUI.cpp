#include "PedalboardUI.h"

PedalboardUI pedalboardUI;

PedalboardUI::PedalboardUI() {
}

void PedalboardUI::begin() {
    display.clearScreen(BLACK);
    
    // Header
    display.drawCenteredText(10, "MIDI CONTROLLER", CYAN, BLACK, 2);
    display.drawHLine(0, 30, display.getWidth(), DARKGRAY);
    
    // Initial Bank Label
    updateBankLabel("Bank 1");
    
    // Draw initial button boxes (unpressed)
    for (int i = 0; i < 4; i++) {
        drawButtonBox(i, false);
    }
}

void PedalboardUI::setButtonState(uint8_t index, bool pressed) {
    if (index >= 4) return;
    drawButtonBox(index, pressed);
}

void PedalboardUI::updateBankLabel(const String& bankName) {
    // Clear the bank label area (approx y=40 to y=60)
    display.fillRect(0, 40, display.getWidth(), 20, BLACK);
    display.drawCenteredText(45, bankName, YELLOW, BLACK, 2);
}

void PedalboardUI::showStatusMessage(const String& msg, uint16_t color) {
    // Status area at the bottom
    int y = 150;
    display.clearCenteredLine(y, 1, BLACK);
    display.drawCenteredText(y, msg, color, BLACK, 1);
}

void PedalboardUI::drawButtonBox(uint8_t index, bool pressed) {
    // Calculate position to center the group of 4 buttons
    // Total width = 4 * width + 3 * gap
    // Start X = (Screen Width - Total Width) / 2
    
    int totalWidth = (4 * BUTTON_BOX_WIDTH) + (3 * BUTTON_GAP);
    int startX = (display.getWidth() - totalWidth) / 2;
    
    int x = startX + index * (BUTTON_BOX_WIDTH + BUTTON_GAP);
    int y = BUTTON_Y_POS;
    
    uint16_t fillColor = pressed ? GREEN : BLACK;
    uint16_t borderColor = pressed ? WHITE : GRAY;
    uint16_t textColor = pressed ? BLACK : WHITE;
    
    // Draw box
    display.fillRoundRect(x, y, BUTTON_BOX_WIDTH, BUTTON_BOX_HEIGHT, 5, fillColor);
    display.drawRoundRect(x, y, BUTTON_BOX_WIDTH, BUTTON_BOX_HEIGHT, 5, borderColor);
    
    // Draw number
    String label = String(index + 1);
    int textX = x + (BUTTON_BOX_WIDTH - display.getTextWidth(label, 2)) / 2;
    int textY = y + (BUTTON_BOX_HEIGHT - display.getCharHeight(2)) / 2;
    
    display.drawText(textX, textY, label, textColor, fillColor, 2);
}
