#ifndef PEDALBOARD_UI_H
#define PEDALBOARD_UI_H

#include "ST7789_Graphics.h"

class PedalboardUI {
public:
    PedalboardUI();
    
    void begin();
    void update(); // Vacío - sin animaciones
    void setButtonState(uint8_t index, bool state, uint8_t buttonType = 0);
    void updateBankLabel(const String& bankName);
    void showStatusMessage(const String& msg, uint16_t color = GREEN);

private:
    void drawToggleButton(uint8_t index, bool state);
};

extern PedalboardUI pedalboardUI;

#endif // PEDALBOARD_UI_H
