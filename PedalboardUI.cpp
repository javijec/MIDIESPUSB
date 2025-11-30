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
    
    // Draw initial pedals (unpressed)
    for (int i = 0; i < 4; i++) {
        drawPedal(i, false);
    }
}

void PedalboardUI::update() {
    // Check status message timeout
    if (messageVisible && (millis() - messageStartTime > MESSAGE_DURATION)) {
        messageVisible = false;
        // Clear status area
        int y = 150;
        display.fillRect(0, y - 10, display.getWidth(), 25, BLACK);
    }
}

void PedalboardUI::setButtonState(uint8_t index, bool pressed, uint8_t buttonType) {
    if (index >= 4) return;
    
    // Draw based on button type
    if (buttonType == 1) { // BUTTON_TOGGLE
        drawToggleButton(index, pressed);
    } else { // BUTTON_MOMENTARY
        drawPedal(index, pressed);
    }
}

void PedalboardUI::updateBankLabel(const String& bankName) {
    // Clear the bank label area (approx y=40 to y=60)
    display.fillRect(0, 40, display.getWidth(), 20, BLACK);
    display.drawCenteredText(45, bankName, YELLOW, BLACK, 2);
}

void PedalboardUI::showStatusMessage(const String& msg, uint16_t color) {
    // Status area at the bottom
    int y = 150;
    
    // Draw background bar
    display.fillRect(0, y - 10, display.getWidth(), 25, DARKGRAY);
    
    // Draw text
    display.drawCenteredText(y, msg, color, DARKGRAY, 1);
    
    // Set timer
    messageStartTime = millis();
    messageVisible = true;
}

void PedalboardUI::drawPedal(uint8_t index, bool pressed) {
    // Calculate position to center the group of 4 pedals
    // Total width = 4 * (2*radius) + 3 * gap
    int diameter = 2 * PEDAL_RADIUS;
    int totalWidth = (4 * diameter) + (3 * PEDAL_GAP);
    int startX = (display.getWidth() - totalWidth) / 2;
    
    // Center of the pedal
    int cx = startX + index * (diameter + PEDAL_GAP) + PEDAL_RADIUS;
    int cy = PEDAL_Y_POS;
    
    // Colors
    uint16_t ringColor = LIGHTGRAY;
    uint16_t bodyColor = pressed ? CYAN : DARKGRAY;
    uint16_t actuatorColor = pressed ? WHITE : GRAY;
    uint16_t textColor = WHITE;
    
    // Draw Outer Ring (Metallic look)
    display.fillCircle(cx, cy, PEDAL_RADIUS, ringColor);
    display.drawCircle(cx, cy, PEDAL_RADIUS, WHITE); // Highlight
    
    // Draw Inner Body (The part that lights up)
    display.fillCircle(cx, cy, PEDAL_RADIUS - 4, bodyColor);
    
    // Draw Center Actuator (The physical switch)
    display.fillCircle(cx, cy, PEDAL_RADIUS - 12, actuatorColor);
    display.drawCircle(cx, cy, PEDAL_RADIUS - 12, BLACK);
    
    // Draw Number below the pedal
    String label = String(index + 1);
    int textY = cy + PEDAL_RADIUS + 10;
    display.drawCenteredText(textY, label, textColor, BLACK, 1);
}

void PedalboardUI::drawToggleButton(uint8_t index, bool state) {
    // Calculate position - same as pedals but rectangular
    const int BTN_WIDTH = 50;
    const int BTN_HEIGHT = 50;
    int diameter = 2 * PEDAL_RADIUS;
    int totalWidth = (4 * diameter) + (3 * PEDAL_GAP);
    int startX = (display.getWidth() - totalWidth) / 2;
    
    int x = startX + index * (diameter + PEDAL_GAP) + (PEDAL_RADIUS - BTN_WIDTH/2);
    int y = PEDAL_Y_POS - BTN_HEIGHT/2;
    
    // Colors for ON/OFF states
    uint16_t fillColor = state ? GREEN : DARKGRAY;
    uint16_t borderColor = state ? WHITE : GRAY;
    uint16_t textColor = state ? BLACK : WHITE;
    
    // Draw button
    display.fillRoundRect(x, y, BTN_WIDTH, BTN_HEIGHT, 5, fillColor);
    display.drawRoundRect(x, y, BTN_WIDTH, BTN_HEIGHT, 5, borderColor);
    
    // Draw label
    String label = state ? "ON" : "OFF";
    int textX = x + (BTN_WIDTH - display.getTextWidth(label, 2)) / 2;
    int textY = y + (BTN_HEIGHT - display.getCharHeight(2)) / 2;
    display.drawText(textX, textY, label, textColor, fillColor, 2);
    
    // Draw number below
    String numLabel = String(index + 1);
    int numY = y + BTN_HEIGHT + 10;
    display.drawCenteredText(numY, numLabel, WHITE, BLACK, 1);
}
