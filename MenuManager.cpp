#include "MenuManager.h"
#include "PedalboardUI.h"
#include "ConfigManager.h"


MenuManager menuManager;

// Global variables for menu settings (if not in ConfigManager)
int globalBrightness = 255;
int bleEnabled = 1;

MenuManager::MenuManager() {
}

void MenuManager::begin() {
    buildMenu();
}

void MenuManager::buildMenu() {
    items.clear();
    
    // 1. Bank Select
    items.push_back({
        "Bank", 
        MENU_ITEM_VALUE, 
        nullptr, // We'll handle value manually in callback or getter
        0, 3, 
        [](MenuManager* mgr) { 
            configManager.nextBank(); 
        }
    });
    
    // 2. BLE Toggle
    items.push_back({
        "BLE", 
        MENU_ITEM_TOGGLE, 
        &bleEnabled, 
        0, 1, 
        [](MenuManager* mgr) {
            // Toggle logic here if needed beyond just changing the int
            // BLEDevice::deinit() is hard to recover from, maybe just stop advertising?
        }
    });
    
    // 3. Brightness (Placeholder)
    items.push_back({
        "Brightness",
        MENU_ITEM_VALUE,
        &globalBrightness,
        0, 255,
        nullptr
    });
    
    // 4. Save & Exit
    items.push_back({
        "Exit", 
        MENU_ITEM_ACTION, 
        nullptr, 
        0, 0, 
        [](MenuManager* mgr) { mgr->close(); }
    });
}

bool MenuManager::isActive() {
    return active;
}

void MenuManager::open() {
    active = true;
    selectedIndex = 0;
    pedalboardUI.drawMenu(); // Initial draw
}

void MenuManager::close() {
    active = false;
    
    // Restore main UI background (Header, etc.)
    pedalboardUI.redraw();
    
    // Force redraw of main interface
    pedalboardUI.updateBankLabel("Bank " + String(configManager.getCurrentBank() + 1));
    for (int i = 0; i < 4; i++) {
        MidiButtonConfig cfg = configManager.getButtonConfig(i);
        pedalboardUI.setButtonState(i, false, cfg.type);
    }
}

void MenuManager::handleButton(uint8_t logicalId, uint8_t eventType) {
    if (!active) return;
    
    // Simple navigation: Press to move/select
    if (eventType == 1) { // PRESSED (using raw value 1 from ButtonManager.h or we should include it)
        // We need to know the event types. Assuming 1 is Pressed.
        // Let's check ButtonManager.h or assume standard
        
        switch (logicalId) {
            case 0: // Button 1: Up
                moveUp();
                break;
            case 1: // Button 2: Down
                moveDown();
                break;
            case 2: // Button 3: Select / Toggle
                select();
                break;
            case 3: // Button 4: Back
                back();
                break;
        }
        if (active) {
            pedalboardUI.drawMenu(); // Redraw after action
        }
    }
}

void MenuManager::moveUp() {
    if (selectedIndex > 0) {
        selectedIndex--;
    } else {
        selectedIndex = items.size() - 1; // Wrap around
    }
}

void MenuManager::moveDown() {
    if (selectedIndex < items.size() - 1) {
        selectedIndex++;
    } else {
        selectedIndex = 0; // Wrap around
    }
}

void MenuManager::select() {
    MenuItem& item = items[selectedIndex];
    
    if (item.callback) {
        item.callback(this);
    }
    
    if (item.type == MENU_ITEM_TOGGLE && item.valuePtr) {
        *item.valuePtr = !(*item.valuePtr);
    }
}

void MenuManager::back() {
    close();
}

String MenuManager::getTitle() {
    return "Menu";
}

int MenuManager::getItemCount() {
    return items.size();
}

String MenuManager::getItemLabel(int index) {
    if (index >= 0 && index < items.size()) {
        return items[index].label;
    }
    return "";
}

String MenuManager::getItemValueStr(int index) {
    if (index >= 0 && index < items.size()) {
        MenuItem& item = items[index];
        if (item.label == "Bank") {
            return String(configManager.getCurrentBank() + 1);
        }
        if (item.type == MENU_ITEM_TOGGLE && item.valuePtr) {
            return (*item.valuePtr) ? "ON" : "OFF";
        }
        if (item.type == MENU_ITEM_VALUE && item.valuePtr) {
            return String(*item.valuePtr);
        }
    }
    return "";
}

int MenuManager::getSelectedIndex() {
    return selectedIndex;
}
