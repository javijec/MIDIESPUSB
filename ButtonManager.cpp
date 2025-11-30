#include "ButtonManager.h"

// Initialize static members
AceButton ButtonManager::b0(nullptr, 0);
AceButton ButtonManager::b1(nullptr, 1);
AceButton ButtonManager::b2(nullptr, 2);
AceButton ButtonManager::b3(nullptr, 3);

AceButton* const ButtonManager::BUTTONS[ButtonManager::NUM_BUTTONS] = {
    &ButtonManager::b0, 
    &ButtonManager::b1, 
    &ButtonManager::b2, 
    &ButtonManager::b3
};

// ADC levels for the ladder buttons
const uint16_t ButtonManager::LEVELS[ButtonManager::NUM_LEVELS] = {
  800,  /* 0%, short to ground */
  1800, /* 32%, 4.7 kohm */
  2800, /* 50%, 10 kohm */
  3800, /* 82%, 47 kohm */
  4095, /* 100%, 10-bit ADC, open circuit */
};

LadderButtonConfig ButtonManager::buttonConfig(
  ButtonManager::BUTTON_PIN, 
  ButtonManager::NUM_LEVELS, 
  ButtonManager::LEVELS, 
  ButtonManager::NUM_BUTTONS, 
  ButtonManager::BUTTONS
);

ButtonCallback ButtonManager::userCallback = nullptr;

ButtonManager::ButtonManager() {
    // Constructor
}

void ButtonManager::begin(ButtonCallback callback) {
    userCallback = callback;

    pinMode(BUTTON_PIN, INPUT);

    // Configure the ButtonConfig
    buttonConfig.setEventHandler(handleEvent);
    buttonConfig.setFeature(ButtonConfig::kFeatureClick);
    buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
    buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
    buttonConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
}

void ButtonManager::update() {
    buttonConfig.checkButtons();
}

void ButtonManager::handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    if (userCallback) {
        userCallback(button->getPin(), eventType);
    }
}
