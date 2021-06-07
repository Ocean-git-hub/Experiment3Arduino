#include <Arduino.h>
#include <DHT.h>

#define DHT11_2_PIN 2
#define DHT11_3_PIN 3

DHT dht11_2(DHT11_2_PIN, DHT11);
DHT dht11_3(DHT11_3_PIN, DHT11);

void setup() {
    dht11_2.begin();
    dht11_3.begin();

    Serial.begin(9600);
    Serial.println("2_temp,2_hum,3_temp,3_hum");
}

void loop() {
    delay(2100);
    Serial.print(dht11_2.readTemperature());
    Serial.print(",");
    Serial.print(dht11_2.readHumidity());
    Serial.print(",");
    Serial.print(dht11_3.readTemperature());
    Serial.print(",");
    Serial.println(dht11_3.readHumidity());
}