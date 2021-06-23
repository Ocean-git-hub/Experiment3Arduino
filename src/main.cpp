#include <Arduino.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include "LCDMenu.h"

#define ESP_SERIAL_SPEED 9600
#define PC_SERIAL_SPEED 9600

#define LCD_COLS 16
#define LCD_LINES 2

#define LCD_RS_PIN 19
#define LCD_ENABLE_PIN 18
#define LCD_D4_PIN 17
#define LCD_D5_PIN 16
#define LCD_D6_PIN 15
#define LCD_D7_PIN 14

#define BOX_LID_PIN 13

#define SWITCH_SELECT_PIN 12
#define SWITCH_RIGHT_PIN 11
#define SWITCH_LEFT_PIN 10
#define SWITCH_UP_PIN 9
#define SWITCH_DOWN_PIN 8

#define DHT11_OUTSIDE_PIN 7
#define DHT11_INSIDE_PIN 6

#define ESP_SERIAL_RX_PIN 5
#define ESP_SERIAL_TX_PIN 4

#define ULTRAVIOLET_RAYS_PIN 3

#define FAN_DRIVE_PIN 2

DHT dht11Outside(DHT11_OUTSIDE_PIN, DHT11);
DHT dht11Inside(DHT11_INSIDE_PIN, DHT11);

LCDMenu lcdMenu(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

SoftwareSerial espSerial(ESP_SERIAL_RX_PIN, ESP_SERIAL_TX_PIN);

struct ESPTime {
    uint32_t tm_sec;
    uint32_t tm_min;
    uint32_t tm_hour;
    uint32_t tm_mday;
    uint32_t tm_mon;
    uint32_t tm_year;
    uint32_t tm_wday;
    uint32_t tm_yday;
    uint32_t tm_isdst;
};

void initPinMode() {
    pinMode(BOX_LID_PIN, INPUT_PULLUP);

    pinMode(SWITCH_SELECT_PIN, INPUT_PULLUP);
    pinMode(SWITCH_RIGHT_PIN, INPUT_PULLUP);
    pinMode(SWITCH_LEFT_PIN, INPUT_PULLUP);
    pinMode(SWITCH_UP_PIN, INPUT_PULLUP);
    pinMode(SWITCH_DOWN_PIN, INPUT_PULLUP);

    pinMode(ULTRAVIOLET_RAYS_PIN, OUTPUT);

    pinMode(FAN_DRIVE_PIN, OUTPUT);
}

void initDHT11() {
    dht11Outside.begin();
    dht11Inside.begin();
}

void initSerial() {
    espSerial.begin(ESP_SERIAL_SPEED);
    Serial.begin(PC_SERIAL_SPEED);
}

volatile bool leftPushed = false, rightPushed = false, upPushed = false, downPushed = false,
        selectPushed = false;

ISR(PCINT0_vect) {
    if (digitalRead(SWITCH_LEFT_PIN) == LOW && !leftPushed) {
        leftPushed = true;
        interrupts();
        lcdMenu.operation(OperationType::Left);
        delayMicroseconds(30000);
        while (digitalRead(SWITCH_LEFT_PIN) == LOW);
        delayMicroseconds(30000);
        leftPushed = false;
    } else if (digitalRead(SWITCH_RIGHT_PIN) == LOW && !rightPushed) {
        rightPushed = true;
        interrupts();
        lcdMenu.operation(OperationType::Right);
        delayMicroseconds(30000);
        while (digitalRead(SWITCH_RIGHT_PIN) == LOW);
        delayMicroseconds(30000);
        rightPushed = false;
    } else if (digitalRead(SWITCH_SELECT_PIN) == LOW && !selectPushed) {
        selectPushed = true;
        interrupts();
        lcdMenu.operation(OperationType::Select);
        delayMicroseconds(30000);
        while (digitalRead(SWITCH_SELECT_PIN) == LOW);
        delayMicroseconds(30000);
        selectPushed = false;
    } else if (digitalRead(SWITCH_UP_PIN) == LOW && !upPushed) {
        upPushed = true;
        interrupts();
        lcdMenu.operation(OperationType::Up);
        delayMicroseconds(30000);
        while (digitalRead(SWITCH_UP_PIN) == LOW);
        delayMicroseconds(30000);
        upPushed = false;
    } else if (digitalRead(SWITCH_DOWN_PIN) == LOW && !downPushed) {
        downPushed = true;
        interrupts();
        lcdMenu.operation(OperationType::Down);
        delayMicroseconds(30000);
        while (digitalRead(SWITCH_DOWN_PIN) == LOW);
        delayMicroseconds(30000);
        downPushed = false;
    }
}

void initInterrupt() {
    PCICR |= 1 << PCIE0;
    PCMSK0 |= 0b11111;
}

void readEspTime(ESPTime *espTime) {
    espSerial.write(1);
    while (espSerial.available() <= 0);
    espSerial.readBytes((char *) espTime, sizeof(ESPTime));
}

ESPTime time{};

void updateSecond() {
    interrupts();
    if (++time.tm_sec >= 60) {
        time.tm_sec = 0;
        if (++time.tm_min >= 60)
            readEspTime(&time);
        lcdMenu.setTime(time.tm_hour, time.tm_min);
        lcdMenu.update();
    }
}

void setup() {
    initPinMode();
    initDHT11();
    initSerial();

    lcdMenu.printStartMessage();

    readEspTime(&time);

    lcdMenu.setTime(time.tm_hour, time.tm_min);
    lcdMenu.setWeather(WeatherType::Rain);
    lcdMenu.begin();

    MsTimer2::set(1000, updateSecond);
    MsTimer2::start();

    initInterrupt();
}

void loop() {

}