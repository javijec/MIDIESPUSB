#include "ButtonManager.h"

// Initialize static members
AceButton ButtonManager::dummyButton(nullptr, 0);
AceButton ButtonManager::b0(nullptr, 1);
AceButton ButtonManager::b1(nullptr, 2);
AceButton ButtonManager::b2(nullptr, 3);
AceButton ButtonManager::b3(nullptr, 4);

AceButton* const ButtonManager::BUTTONS[ButtonManager::NUM_BUTTONS] = {
    &ButtonManager::dummyButton,
    &ButtonManager::b0, 
    &ButtonManager::b1, 
    &ButtonManager::b2, 
    &ButtonManager::b3
};

// ADC levels for the ladder buttons
const uint16_t ButtonManager::LEVELS[ButtonManager::NUM_LEVELS] = {
  500,  /* Idle state (0) -> Index 0 */
  1580, /* Button 0 (approx 1340) -> Index 1 (Lowered from 1735) */
  2370, /* Button 1 (approx 2130) -> Index 2 (Lowered from 2525) */
  3170, /* Button 2 (approx 2920) -> Index 3 (Lowered from 3320) */
  3800, /* Button 3 (approx 3720) -> Index 4 (Lowered from 3900) */
  4095, /* 100%, Open circuit */
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
    buttonConfig.setDebounceDelay(50); // Increased from default 20ms to 50ms
}

void ButtonManager::update() {
    buttonConfig.checkButtons();
}

void ButtonManager::handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
    if (userCallback) {
        userCallback(button->getPin(), eventType);
    }
}
