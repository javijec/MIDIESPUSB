#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <Arduino.h>
#include <vector>

// Forward declaration
class MenuManager;

// Menu Item Types
enum MenuItemType {
    MENU_ITEM_ACTION,
    MENU_ITEM_TOGGLE,
    MENU_ITEM_VALUE,
    MENU_ITEM_SUBMENU
};

// Callback function type
typedef void (*MenuCallback)(MenuManager* mgr);

struct MenuItem {
    String label;
    MenuItemType type;
    int* valuePtr;      // Pointer to value to modify (optional)
    int minValue;
    int maxValue;
    MenuCallback callback; // Action to run when selected
};

class MenuManager {
public:
    MenuManager();
    
    void begin();
    
    // State
    bool isActive();
    void open();
    void close();
    
    // Navigation
    void handleButton(uint8_t logicalId, uint8_t eventType);
    void moveUp();
    void moveDown();
    void select();
    void back();
    
    // Drawing helpers
    String getTitle();
    int getItemCount();
    String getItemLabel(int index);
    String getItemValueStr(int index);
    int getSelectedIndex();
    
    // Actions
    static void toggleBLE(MenuManager* mgr);
    static void nextBank(MenuManager* mgr);
    static void resetConfig(MenuManager* mgr);

private:
    bool active = false;
    int selectedIndex = 0;
    std::vector<MenuItem> items;
    
    void buildMenu();
};

extern MenuManager menuManager;

#endif // MENU_MANAGER_H
