#include <WiFiNINA.h>
#include <NeoPixelConnect.h>
#define NEOPIXELS_CONTROL_PIN 15
#define NUM_NEOPIXELS 5
#define EXINTERRUPT_PIN 4
#define ON HIGH
#define OFF LOW

bool flgSensorDetectionChk = false;
bool flgSensorDetectionStatus = false;
bool flgSensorReleasedStatus = false;
bool flgExternalInterruptDetected = false;
unsigned long SensorChkTimeInterval = 200;
unsigned long SensorChkTimePreviousMillis = 0;

void mySensorStatusCheckHandler();
void onInterruptTriggerChangeHandler();

NeoPixelConnect p(NEOPIXELS_CONTROL_PIN, NUM_NEOPIXELS, pio0, 0);
void setup()
{
    Serial.begin(9600);
    pinMode(EXINTERRUPT_PIN, INPUT_PULLUP);
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
    Serial.println("External Interrupt Test using PIR Sensor");
    attachInterrupt(EXINTERRUPT_PIN, onInterruptTriggerChangeHandler, CHANGE);
    p.neoPixelInit(NEOPIXELS_CONTROL_PIN, NUM_NEOPIXELS);
    p.neoPixelFill(0, 0, 0, true);
    delay(500);
    p.neoPixelFill(255, 0, 0, true);
    delay(500);
    p.neoPixelFill(0, 255, 0, true);
    delay(500);
    p.neoPixelFill(0, 0, 255, true);
    delay(500);
    p.neoPixelFill(255, 255, 255, true);
    delay(500);
    p.neoPixelFill(0, 0, 0, true);
    delay(500);
}
void loop() { mySensorStatusCheckHandler(); }

void mySensorStatusCheckHandler()
{
    if (!flgExternalInterruptDetected)
        return;
    if ((unsigned long)(millis() - SensorChkTimePreviousMillis) >= SensorChkTimeInterval)
    {
        if (flgSensorDetectionChk)
        {
            if (!flgSensorDetectionStatus)
            {
                flgSensorDetectionStatus = true;
                flgSensorReleasedStatus = false;
                Serial.println("PIR Sensor Activated");
                digitalWrite(LEDR, ON);
                digitalWrite(LEDG, OFF);
                p.neoPixelFill(100, 0, 0, true);
                flgExternalInterruptDetected = false;
            }
        }
        else
        {
            if (!flgSensorReleasedStatus)
            {
                flgSensorReleasedStatus = true;
                flgSensorDetectionStatus = false;
                Serial.println("PIR Sensor Released");
                digitalWrite(LEDR, OFF);
                digitalWrite(LEDG, ON);
                p.neoPixelFill(0, 100, 0, true);
                flgExternalInterruptDetected = false;
            }
        }
        SensorChkTimePreviousMillis = millis();
    }
}
void onInterruptTriggerChangeHandler()
{
    if (digitalRead(EXINTERRUPT_PIN))
    {
        flgSensorDetectionChk = true;
        flgExternalInterruptDetected = true;
    }
    else
    {
        flgSensorDetectionChk = false;
        flgExternalInterruptDetected = true;
    }
}
