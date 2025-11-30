#line 1 "C:\\Users\\javij\\OneDrive\\01-Programacion\\06-Arduino\\Pedalera MIDI\\MIDIESPUSB\\Button_Driver.h"
#include <AceButton.h>
using namespace ace_button;

static const uint8_t BUTTON_PIN = 2;

// Create 4 AceButton objects, with their virtual pin number 0 to 3. The number
// of buttons does not need to be identical to the number of analog levels. You
// can choose to define only a subset of buttons here.
//
// Use the 4-parameter `AceButton()` constructor with the `buttonConfig`
// parameter explicitly to `nullptr` to prevent the automatic creation of the
// default SystemButtonConfig, saving about 30 bytes of flash and 26 bytes of
// RAM on an AVR processor.
static const uint8_t NUM_BUTTONS = 4;
static AceButton b0(nullptr, 0);
static AceButton b1(nullptr, 1);
static AceButton b2(nullptr, 2);
static AceButton b3(nullptr, 3);
static AceButton* const BUTTONS[NUM_BUTTONS] = {
    &b0, &b1, &b2, &b3,
};

// Define the ADC voltage levels for each button.
// For 4 buttons, we need 5 levels.
static const uint8_t NUM_LEVELS = NUM_BUTTONS + 1;
static const uint16_t LEVELS[NUM_LEVELS] = {
  800 /* 0%, short to ground */,
  1800 /* 32%, 4.7 kohm */,
  2800 /* 50%, 10 kohm */,
  3800 /* 82%, 47 kohm */,
  4095 /* 100%, 10-bit ADC, open circuit */,
};

// The LadderButtonConfig constructor binds the AceButton to the
// LadderButtonConfig.
static LadderButtonConfig buttonConfig(
  BUTTON_PIN, NUM_LEVELS, LEVELS, NUM_BUTTONS, BUTTONS
);

// The event handler for the button.
void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {

  // Print out a message for all events.
  Serial.print(F("handleEvent(): "));
  Serial.print(F("virtualPin: "));
  Serial.print(button->getPin());
  Serial.print(F("; eventType: "));
  Serial.print(eventType);
  Serial.print(F("; buttonState: "));
  Serial.println(buttonState);
}

void setup() {
  delay(1000); // some microcontrollers reboot twice
  Serial.begin(115200);
  while (! Serial); // Wait until Serial is ready - Leonardo/Micro

  // Don't use internal pull-up resistor.
  pinMode(BUTTON_PIN, INPUT);

  // Configure the ButtonConfig with the event handler, and enable all higher
  // level events.
  buttonConfig.setEventHandler(handleEvent);
  buttonConfig.setFeature(ButtonConfig::kFeatureClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig.setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig.setFeature(ButtonConfig::kFeatureRepeatPress);
}

// The buttonConfig.checkButtons() should be called every 4-5ms or faster, if
// the debouncing time is ~20ms. On ESP8266, analogRead() must be called *no*
// faster than 4-5ms to avoid a bug which disconnects the WiFi connection.
void checkButtons() {
  static unsigned long prev = millis();

  // DO NOT USE delay(5) to do this.
  unsigned long now = millis();
  if (now - prev > 5) {
    buttonConfig.checkButtons();
    prev = now;
  }
}

void loop() {
  checkButtons();
}