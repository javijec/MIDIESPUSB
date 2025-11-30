#ifndef PEDALBOARD_UI_H
#define PEDALBOARD_UI_H

#include "ST7789_Graphics.h"

class PedalboardUI {
public:
    PedalboardUI();
    
    // Initialize and draw the static interface elements
    void begin();
    
    // Main UI loop for animations and timeouts
    void update();

    // Update the visual state of a button (pressed/released)
    void setButtonState(uint8_t index, bool pressed);
    
    // Update the displayed bank name
    void updateBankLabel(const String& bankName);
    
    // Show a temporary status message (like "Note Sent")
    void showStatusMessage(const String& msg, uint16_t color = GREEN);

private:
    // Layout constants
    static const int PEDAL_RADIUS = 25;
    static const int PEDAL_GAP = 15;
    static const int PEDAL_Y_POS = 110;
    
    // Status message state
    unsigned long messageStartTime = 0;
    bool messageVisible = false;
    static const int MESSAGE_DURATION = 2000;

    // Helper to draw a single virtual pedal
    void drawPedal(uint8_t index, bool pressed);
};

extern PedalboardUI pedalboardUI;

#endif // PEDALBOARD_UI_H
