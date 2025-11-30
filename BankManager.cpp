#include "BankManager.h"

BankManager bankManager;

BankManager::BankManager() {
    currentBankIndex = 0;
}

void BankManager::begin() {
    // Define Bank 1 (Basic Notes)
    banks[0].name = "Bank 1: Basic";
    banks[0].notes[0] = { MIDI_Notes::C[4], Channel_1 };
    banks[0].notes[1] = { MIDI_Notes::D[4], Channel_1 };
    banks[0].notes[2] = { MIDI_Notes::E[4], Channel_1 };
    banks[0].notes[3] = { MIDI_Notes::F[4], Channel_1 };
    
    // Define Bank 2 (Higher Octave)
    banks[1].name = "Bank 2: High";
    banks[1].notes[0] = { MIDI_Notes::G[4], Channel_1 };
    banks[1].notes[1] = { MIDI_Notes::A[4], Channel_1 };
    banks[1].notes[2] = { MIDI_Notes::B[4], Channel_1 };
    banks[1].notes[3] = { MIDI_Notes::C[5], Channel_1 };
    
    // Define Bank 3 (Chords/Specials - placeholders)
    banks[2].name = "Bank 3: Special";
    banks[2].notes[0] = { MIDI_Notes::C[3], Channel_1 };
    banks[2].notes[1] = { MIDI_Notes::E[3], Channel_1 };
    banks[2].notes[2] = { MIDI_Notes::G[3], Channel_1 };
    banks[2].notes[3] = { MIDI_Notes::B[3], Channel_1 };
}

void BankManager::nextBank() {
    currentBankIndex++;
    if (currentBankIndex >= NUM_BANKS) {
        currentBankIndex = 0;
    }
}

String BankManager::getCurrentBankName() {
    return banks[currentBankIndex].name;
}

MIDIAddress BankManager::getNoteForButton(uint8_t buttonIndex) {
    if (buttonIndex >= BUTTONS_PER_BANK) {
        return { MIDI_Notes::C[4], Channel_1 }; // Default fallback
    }
    return banks[currentBankIndex].notes[buttonIndex];
}
