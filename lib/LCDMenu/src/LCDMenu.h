#ifndef ARDUINO_PROJECT_LCDMENU_H
#define ARDUINO_PROJECT_LCDMENU_H

#include <LiquidCrystal.h>

#define MENU_NUMS 3

enum WeatherType {
    Sun,
    Cloud,
    Rain,
    Snow,
    Other
};

enum OperationType {
    Right,
    Left,
    Up,
    Down,
    Select,
    None
};

enum MenuType {
    SelectMenu,
    SelectedMenu
};

class LCDMenu {
    LiquidCrystal lcd;
    unsigned char menuIndex = 0;
    const char *menu[MENU_NUMS] = {
            " CheckLifeTime >",
            "< Set LifeTime >",
            "< ResetLifeTime "
    };
    unsigned char defaultLifeTimeDay = 0;
    unsigned char lifeTimeDay = 0;
    MenuType menuType = MenuType::SelectMenu;
    WeatherType weather = WeatherType::Other;
    uint8_t statusFiveBits = 1 << 4;
    uint8_t hour = 0;
    uint8_t min = 0;

    void init();

    void clearLine(unsigned char line);

    void nextMenu();

    void previousMenu();

    void operationInMenu(OperationType operationType);

    void printLifeTime();

    void setLifeTime(OperationType operationType);

    void resetLifeTime();

    void printMenu();

    void printTime();

    void printWeather();

    void printStatus();

public:
    LCDMenu(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);

    void begin();

    bool print(const char *str);

    void setTime(uint8_t currentHour, uint8_t currentMin);

    void setWeather(WeatherType weatherType);

    void setStatus(uint8_t statusNum);

    void decreaseLifeTime();

    void update();

    void operation(OperationType operationType);

    void clearStatus(uint8_t statusNum);
};

#endif //ARDUINO_PROJECT_LCDMENU_H
