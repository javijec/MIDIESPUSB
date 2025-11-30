#ifndef BANK_MANAGER_H
#define BANK_MANAGER_H

#include <Control_Surface.h>

class BankManager {
public:
    BankManager();
    
    // Initialize banks
    void begin();
    
    // Switch to the next bank
    void nextBank();
    
    // Get the name of the current bank
    String getCurrentBankName();
    
    // Get the MIDI note for a specific button in the current bank
    MIDIAddress getNoteForButton(uint8_t buttonIndex);

private:
    static const uint8_t NUM_BANKS = 3;
    static const uint8_t BUTTONS_PER_BANK = 4;
    
    uint8_t currentBankIndex;
    
    struct Bank {
        String name;
        MIDIAddress notes[BUTTONS_PER_BANK];
    };
    
    Bank banks[NUM_BANKS];
};

extern BankManager bankManager;

#endif // BANK_MANAGER_H
