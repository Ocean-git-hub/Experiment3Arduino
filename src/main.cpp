#include <Arduino.h>
#include <DHT.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#include <TimeAlarms.h>
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

#define ULTRAVIOLET_RAYS_LED_DRIVE_PIN 3

#define FAN_DRIVE_PIN 2

#define CHATTERING_DELAY 10000

volatile bool isSwitchPushed = false;

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

    pinMode(ULTRAVIOLET_RAYS_LED_DRIVE_PIN, OUTPUT);

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

AlarmId ultravioletDriveTimer, ultravioletRayTimer;

void offUltraviolet() {
    digitalWrite(ULTRAVIOLET_RAYS_LED_DRIVE_PIN, LOW);
    lcdMenu.clearStatus(2);
    lcdMenu.update();
}

void driveUltraviolet() {
    digitalWrite(ULTRAVIOLET_RAYS_LED_DRIVE_PIN, HIGH);
    Alarm.free(ultravioletRayTimer);
    Alarm.timerOnce(11, 0, 0, offUltraviolet);
    lcdMenu.setStatus(2);
    lcdMenu.update();
}

volatile uint8_t isBoxLidOpen;

ISR(PCINT0_vect) {
    uint8_t isBoxOpening = digitalRead(BOX_LID_PIN) == LOW ? 0 : 1;

    if ((isBoxLidOpen ^ isBoxOpening) == 1) {
        offUltraviolet();
        Alarm.free(ultravioletDriveTimer);
        if (isBoxLidOpen && !isBoxOpening) {
            lcdMenu.setStatus(1);
            ultravioletDriveTimer = Alarm.timerOnce(5, driveUltraviolet);
        } else
            lcdMenu.clearStatus(1);
        isBoxLidOpen = isBoxOpening;
        lcdMenu.update();
    }

    if (!isSwitchPushed) {
        int pushedSwitchPin = -1;
        if (digitalRead(SWITCH_RIGHT_PIN) == LOW)
            pushedSwitchPin = SWITCH_RIGHT_PIN;
        else if (digitalRead(SWITCH_LEFT_PIN) == LOW)
            pushedSwitchPin = SWITCH_LEFT_PIN;
        else if (digitalRead(SWITCH_SELECT_PIN) == LOW)
            pushedSwitchPin = SWITCH_SELECT_PIN;
        else if (digitalRead(SWITCH_UP_PIN) == LOW)
            pushedSwitchPin = SWITCH_UP_PIN;
        else if (digitalRead(SWITCH_DOWN_PIN) == LOW)
            pushedSwitchPin = SWITCH_DOWN_PIN;

        if (pushedSwitchPin != -1) {
            isSwitchPushed = true;
            interrupts();
            switch (pushedSwitchPin) {
                case SWITCH_RIGHT_PIN:
                    lcdMenu.operation(OperationType::Right);
                    break;
                case SWITCH_LEFT_PIN:
                    lcdMenu.operation(OperationType::Left);
                    break;
                case SWITCH_UP_PIN:
                    lcdMenu.operation(OperationType::Up);
                    break;
                case SWITCH_DOWN_PIN:
                    lcdMenu.operation(OperationType::Down);
                    break;
                case SWITCH_SELECT_PIN:
                    lcdMenu.operation(OperationType::Select);
                    break;
                default:;
            }
            delayMicroseconds(CHATTERING_DELAY);
            while (digitalRead(pushedSwitchPin) == LOW);
            delayMicroseconds(CHATTERING_DELAY);
            isSwitchPushed = false;
        }
    }
}

void initInterrupt() {
    PCICR |= 1 << PCIE0;
    PCMSK0 |= 0b111111;
}

ESPTime time{};

void readEspTime() {
//    espSerial.write(1);
//    while (espSerial.available() <= 0);
//    espSerial.readBytes((char *) &time, sizeof(ESPTime));
    setTime(time.tm_hour, time.tm_min, time.tm_sec, time.tm_mday, time.tm_mon, time.tm_year % 100);
}

void updateSecond() {
    interrupts();
    Alarm.delay(0);
}

void updateLcdTime() {
    lcdMenu.setTime(hour(), minute());
    lcdMenu.update();
}

int volumetricHumidity(float temp, float humidity) {
    return 6.1078 * pow(10, 7.5 * temp / (temp + 237.3)) * 217 / (temp + 273.15) * humidity / 100 + 0.5;
}

void updateDHT() {
    if (volumetricHumidity(dht11Inside.readTemperature(), dht11Inside.readHumidity())
        - volumetricHumidity(dht11Outside.readTemperature(), dht11Outside.readHumidity()) >= 3) {
        digitalWrite(FAN_DRIVE_PIN, HIGH);
        lcdMenu.setStatus(0);
    } else {
        digitalWrite(FAN_DRIVE_PIN, LOW);
        lcdMenu.clearStatus(0);
    }
    lcdMenu.update();
}

void decreaseDay() {
    lcdMenu.decreaseLifeTime();
}

void setup() {
    initPinMode();
    initDHT11();
    initSerial();

    isBoxLidOpen = digitalRead(BOX_LID_PIN) == LOW ? 0 : 1;

    lcdMenu.printStartMessage();

    readEspTime();

    lcdMenu.setTime(hour(), minute());
    lcdMenu.setWeather(WeatherType::Rain);
    lcdMenu.begin();

    MsTimer2::set(1000, updateSecond);
    MsTimer2::start();

    Alarm.timerRepeat(60, updateLcdTime);
    Alarm.timerRepeat(1, 0, 0, readEspTime);
    Alarm.timerRepeat(2, updateDHT);
    Alarm.timerRepeat(0, 0, 1, decreaseDay);

    initInterrupt();
}

void loop() {

}