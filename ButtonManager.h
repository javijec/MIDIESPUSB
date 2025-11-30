#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <AceButton.h>
using namespace ace_button;

// Callback type for button events
// id: button index (0-3)
// eventType: AceButton event type (kEventPressed, etc.)
typedef void (*ButtonCallback)(uint8_t id, uint8_t eventType);

class ButtonManager {
public:
    ButtonManager();
    
    void begin(ButtonCallback callback);
    void update();

    // Expose AceButton constants for the main sketch
    static const uint8_t EVENT_PRESSED = AceButton::kEventPressed;
    static const uint8_t EVENT_RELEASED = AceButton::kEventReleased;
    static const uint8_t EVENT_CLICKED = AceButton::kEventClicked;
    static const uint8_t EVENT_DOUBLE_CLICKED = AceButton::kEventDoubleClicked;
    static const uint8_t EVENT_LONG_PRESSED = AceButton::kEventLongPressed;

private:
    static const uint8_t BUTTON_PIN = 2;
    static const uint8_t NUM_BUTTONS = 4;
    static const uint8_t NUM_LEVELS = NUM_BUTTONS + 1;
    
    // AceButton objects need to be persistent
    static AceButton b0;
    static AceButton b1;
    static AceButton b2;
    static AceButton b3;
    static AceButton* const BUTTONS[NUM_BUTTONS];
    
    static const uint16_t LEVELS[NUM_LEVELS];
    
    static LadderButtonConfig buttonConfig;
    
    // Static wrapper for the library callback
    static void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState);
    
    // User callback
    static ButtonCallback userCallback;
};

#endif // BUTTON_MANAGER_H
