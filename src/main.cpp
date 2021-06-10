#include <Arduino.h>
#include <DHT.h>
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>

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

LiquidCrystal lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN);

SoftwareSerial espSerial(ESP_SERIAL_RX_PIN, ESP_SERIAL_TX_PIN);

void setup() {
    pinMode(BOX_LID_PIN, INPUT_PULLUP);

    pinMode(SWITCH_SELECT_PIN, INPUT_PULLUP);
    pinMode(SWITCH_RIGHT_PIN, INPUT_PULLUP);
    pinMode(SWITCH_LEFT_PIN, INPUT_PULLUP);
    pinMode(SWITCH_UP_PIN, INPUT_PULLUP);
    pinMode(SWITCH_DOWN_PIN, INPUT_PULLUP);

    pinMode(ULTRAVIOLET_RAYS_PIN, OUTPUT);

    pinMode(FAN_DRIVE_PIN, OUTPUT);

    dht11Outside.begin();
    dht11Inside.begin();

    lcd.begin(LCD_COLS, LCD_LINES);

    espSerial.begin(ESP_SERIAL_SPEED);

    Serial.begin(PC_SERIAL_SPEED);
}

void loop() {
    Serial.println("=======================================");

    Serial.print("[Box lid: ");
    Serial.print(digitalRead(BOX_LID_PIN));
    Serial.println("]");

    Serial.print("[Select: ");
    Serial.print(digitalRead(SWITCH_SELECT_PIN));
    Serial.print(", Right: ");
    Serial.print(digitalRead(SWITCH_RIGHT_PIN));
    Serial.print(", Left: ");
    Serial.print(digitalRead(SWITCH_LEFT_PIN));
    Serial.print(", Up: ");
    Serial.print(digitalRead(SWITCH_UP_PIN));
    Serial.print(", Down: ");
    Serial.print(digitalRead(SWITCH_DOWN_PIN));
    Serial.println("]");

    Serial.print("[DHT11 Out[ Temp: ");
    Serial.print(dht11Outside.readTemperature());
    Serial.print(", Humi: ");
    Serial.print((int) dht11Outside.readHumidity());
    Serial.print("] In[ Temp: ");
    Serial.print(dht11Inside.readTemperature());
    Serial.print(", Humi: ");
    Serial.print((int) dht11Inside.readHumidity());
    Serial.println("]]");

    static bool is_ultraviolet_led_on = false;
    is_ultraviolet_led_on = !is_ultraviolet_led_on;
    Serial.print("[Ultraviolet LED: ");
    if (is_ultraviolet_led_on) {
        digitalWrite(ULTRAVIOLET_RAYS_PIN, HIGH);
        Serial.println("ON]");
    } else {
        digitalWrite(ULTRAVIOLET_RAYS_PIN, LOW);

        Serial.println("OFF]");
    }

    static bool is_fan_drive = false;
    is_fan_drive = !is_fan_drive;
    Serial.print("[FAN: ");
    if (is_fan_drive) {
        digitalWrite(FAN_DRIVE_PIN, HIGH);
        Serial.println("ON]");
    } else {
        digitalWrite(FAN_DRIVE_PIN, LOW);

        Serial.println("OFF]");
    }

    espSerial.write(1);
    while (espSerial.available() <= 0);
    String currentTime = espSerial.readString();
    Serial.print("[ESP-8266: [Current time: ");
    Serial.print(currentTime);
    Serial.println("]]");

    lcd.setCursor(0, 0);
    lcd.print(currentTime);

    Serial.println("=======================================");
    Serial.println();

    delay(2100);
}