#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

#define I2C_ADDR 0x55

uint8_t num = 13;
int BuiltInLED = 13;
int numBytes = 13;

// structure to holding the data to be sent
struct Data {
    uint8_t speed;
    uint8_t abs;
    uint8_t airbag;
    uint8_t battery;
    uint8_t checkEngine;
    uint8_t doorOpen;
    uint8_t headlights;
    uint8_t fuelLevel;
    uint8_t temperature;
    uint8_t oillevel;
    uint8_t seatBelt;
    uint8_t sterringWheel;
    uint8_t tirePressure;
} carData;
void onReceive(uint8_t numBytes) {
    Serial.print("RASPI sent: ");
    while (Wire.available()) {
        uint8_t c = Wire.read();
        Serial.print(c);
    }
    Serial.println();
}

void onRequest()
{
    uint8_t buffer[numBytes];
    struct Data carData;

    carData.speed = random(0, 200); // 0 to 200
    carData.abs = (carData.speed > 150) ? 1 : 0; // 1 or 0
    carData.airbag = (carData.speed > 170) ? 1 : 0; // 1 or 0
    carData.battery = buffer[3];                
    carData.checkEngine = (carData.abs == 1 && carData.airbag == 1) ? 1 : 0; // 1 or 0
    carData.doorOpen = (carData.speed < 30) ? 1 : 0; // 1 or 0
    carData.headlights = buffer[6];
    carData.fuelLevel = random(0, 10); // 0 to 10;
    carData.temperature = random(0, 50); // 0 to 50;
    carData.oillevel = random(0, 5); // 0 to 5;
    carData.seatBelt = (carData.speed > 20) ? 1 : 0; // 1 or 0
    carData.sterringWheel = (carData.speed > 50) ? 1 : 0; // 1 or 0
    carData.tirePressure = buffer[12];
    
    buffer[0] = carData.speed;
    buffer[1] = carData.abs;
    buffer[2] = carData.airbag;
    buffer[4] = carData.checkEngine;
    buffer[5] = carData.doorOpen;
    buffer[6] = carData.headlights;
    buffer[7] = carData.fuelLevel;
    buffer[8] = carData.temperature;
    buffer[9] = carData.oillevel;
    buffer[10] = carData.seatBelt;
    buffer[11] = carData.sterringWheel;
    buffer[12] = carData.tirePressure;

    for (int i = 0; i < numBytes; i++) {
        Serial.print(buffer[i]);
        Serial.print(" ");
    }
    Serial.println();

    Wire.write(buffer, numBytes);

    Serial.println("Sent 13 bytes to RPi");
}

void setup() {
    pinMode(A0,INPUT_PULLUP);
    pinMode(BuiltInLED, OUTPUT);
    Serial.begin(9600);
    Wire.begin(I2C_ADDR);

    //Wire.onReceive(onReceive);
    Wire.onRequest(onRequest);

    Serial.println("Arduino I2C Slave Ready @ 0x55");
}

void loop() {
    
}