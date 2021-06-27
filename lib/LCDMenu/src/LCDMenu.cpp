#include "LCDMenu.h"

#define LCD_COLS 16
#define LCD_LINES 2

#define TIME_START_COL 0
#define TIME_START_ROW 0
#define WEATHER_START_COL 6
#define WEATHER_START_ROW 0
#define STATUS_START_COL 11
#define STATUS_START_ROW 0
#define MENU_START_COL 0
#define MENU_START_ROW 1

#define LCD_CHARACTER_CAVITY_SQUARE 1
#define LCD_CHARACTER_DOWN 2
#define LCD_CHARACTER_RETURN 3


LCDMenu::LCDMenu(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
        : lcd(rs, enable, d0, d1, d2, d3) {
    init();
}

void LCDMenu::init() {
    lcd.begin(LCD_COLS, LCD_LINES);
    uint8_t square[8] = {
            0b11111,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b10001,
            0b11111
    };
    uint8_t down[8] = {
            0b00000,
            0b00000,
            0b00000,
            0b00000,
            0b10001,
            0b01010,
            0b00100,
    };
    uint8_t ret[8] = {
            0b11111,
            0b00001,
            0b00101,
            0b01101,
            0b11111,
            0b01100,
            0b00100,
    };
    lcd.createChar(LCD_CHARACTER_CAVITY_SQUARE, square);
    lcd.createChar(LCD_CHARACTER_DOWN, down);
    lcd.createChar(LCD_CHARACTER_RETURN, ret);
    statusFiveBits |= 1 << 4;
}

void LCDMenu::begin() {
    lcd.clear();
    printTime();
    printWeather();
    printStatus();
    printMenu();
}

void LCDMenu::printStartMessage() {
    lcd.setCursor(0, 0);
    lcd.print("Initialized");
}

void LCDMenu::setTime(uint8_t currentHour, uint8_t currentMin) {
    hour = currentHour;
    min = currentMin;
}

void LCDMenu::printTime() {
    lcd.setCursor(TIME_START_COL, TIME_START_ROW);

    char buffer[6] = {};
    sprintf(buffer, "%02d:%02d", hour, min);
    lcd.print(buffer);
}

void LCDMenu::setWeather(WeatherType weatherType) {
    weather = weatherType;
}

void LCDMenu::printWeather() {
    lcd.setCursor(WEATHER_START_COL, WEATHER_START_ROW);

    switch (weather) {
        case Sun:
            lcd.print("Sun");
            break;
        case Cloud:
            lcd.print("Clo.");
            break;
        case Rain:
            lcd.print("Rain");
            break;
        case Snow:
            lcd.print("Snow");
            break;
    }
}

void LCDMenu::setStatus(uint8_t statusNum) {
    if (statusNum < 4)
        statusFiveBits |= 1 << statusNum;
}

void LCDMenu::clearStatus(uint8_t statusNum) {
    if (statusNum < 4)
        statusFiveBits &= ~(1 << statusNum);
}

void LCDMenu::printStatus() {
    lcd.setCursor(STATUS_START_COL, STATUS_START_ROW);
    for (int i = 0; i < 5; ++i) {
        if ((statusFiveBits >> i & 1) == 0)
            lcd.write(LCD_CHARACTER_CAVITY_SQUARE);
        else
            lcd.write(0xff);
    }
}

void LCDMenu::printMenu() {
    lcd.setCursor(MENU_START_COL, MENU_START_ROW);

    if (menuIndex >= 0 && menuIndex < MENU_NUMS)
        lcd.print(menu[menuIndex]);
}

void LCDMenu::update() {
    if (menuType == SelectMenu) {
        clearLine(0);
        printTime();
        printWeather();
        printStatus();
    }
}

void LCDMenu::clearLine(unsigned char line) {
    if (line < LCD_LINES) {
        lcd.setCursor(0, line);
        for (int i = 0; i < LCD_COLS; ++i)
            lcd.print(" ");
    }
}

void LCDMenu::operation(OperationType operationType) {
    switch (menuType) {
        case SelectMenu:
            switch (operationType) {
                case Right:
                    nextMenu();
                    break;
                case Left:
                    previousMenu();
                    break;
                case Select:
                    menuType = SelectedMenu;
                    operation(OperationType::None);
                    break;
                default:
                    break;
            }
            break;
        case SelectedMenu:
            switch (operationType) {
                case Select:
                    menuType = SelectMenu;
                    operationInMenu(operationType);
                    update();
                    printMenu();
                    break;
                default:
                    operationInMenu(operationType);
            }
            break;
    }
}

void LCDMenu::nextMenu() {
    if (menuIndex < MENU_NUMS - 1) {
        menuIndex++;
        printMenu();
    }
}

void LCDMenu::previousMenu() {
    if (menuIndex > 0) {
        menuIndex--;
        printMenu();
    }
}

void LCDMenu::printLifeTime() {
    lcd.clear();
    lcd.setCursor(0, 0);
    char buffer[17] = {};
    sprintf(buffer, "Rem.:%7ddays", lifeTimeDay);
    lcd.print(buffer);
    lcd.setCursor(15, 1);
    lcd.write(LCD_CHARACTER_RETURN);
}

void LCDMenu::setLifeTime(OperationType operationType) {
    switch (operationType) {
        case Up:
            if (lifeTimeDay < 255)
                lifeTimeDay++;
            break;
        case Down:
            if (lifeTimeDay > 0)
                lifeTimeDay--;
            break;
        case Select:
            defaultLifeTimeDay = lifeTimeDay;
            if (lifeTimeDay == 0)
                statusFiveBits |= 1 << 4;
            else
                statusFiveBits &= ~(1 << 4);
            break;
        default:
            break;
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    char buffer[17] = {};
    sprintf(buffer, "Set: %7ddays", lifeTimeDay);
    lcd.print(buffer);
    lcd.setCursor(13, 1);
    lcd.print("^");
    lcd.write(LCD_CHARACTER_DOWN);
    lcd.write(LCD_CHARACTER_RETURN);
}

void LCDMenu::decreaseLifeTime() {
    if (lifeTimeDay > 0)
        lifeTimeDay--;
    if (lifeTimeDay == 0)
        statusFiveBits |= 1 << 4;
}

void LCDMenu::resetLifeTime() {
    lifeTimeDay = defaultLifeTimeDay;
    menuType = SelectMenu;
    if (lifeTimeDay == 0)
        statusFiveBits |= 1 << 4;
    else
        statusFiveBits &= ~(1 << 4);
    update();
}

void LCDMenu::operationInMenu(OperationType operationType) {
    switch (menuIndex) {
        case 0:
            printLifeTime();
            break;
        case 1:
            setLifeTime(operationType);
            break;
        case 2:
            resetLifeTime();
            break;
    }
}

uint8_t LCDMenu::getStatus() {
    return statusFiveBits;
}
