#ifndef PEDALBOARD_UI_H
#define PEDALBOARD_UI_H

#include "ST7789_Graphics.h"

class PedalboardUI {
public:
    PedalboardUI();
    
    // Initialize and draw the static interface elements
    void begin();
    
    // Update the visual state of a button (pressed/released)
    void setButtonState(uint8_t index, bool pressed);
    
    // Update the displayed bank name
    void updateBankLabel(const String& bankName);
    
    // Show a temporary status message (like "Note Sent")
    void showStatusMessage(const String& msg, uint16_t color = GREEN);

private:
    // Layout constants
    static const int BUTTON_BOX_WIDTH = 60;
    static const int BUTTON_BOX_HEIGHT = 40;
    static const int BUTTON_GAP = 10;
    static const int BUTTON_Y_POS = 100;
    
    // Helper to draw a single button box
    void drawButtonBox(uint8_t index, bool pressed);
};

extern PedalboardUI pedalboardUI;

#endif // PEDALBOARD_UI_H
