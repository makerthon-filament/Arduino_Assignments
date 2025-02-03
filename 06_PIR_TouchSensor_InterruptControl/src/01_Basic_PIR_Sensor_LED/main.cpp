#include <WiFiNINA.h>
#define EXINTERRUPT_PIN 4 // D4(GPIO16)
#define ON HIGH
#define OFF LOW
bool flgSensorDetectionChk = false;
bool flgSensorDetectionStatus = false;
bool flgSensorReleasedStatus = false;
bool flgExternalInterruptDetected = false;
unsigned long SensorChkTimeInterval = 200;
unsigned long SensorChkTimePreviousMillis = 0;
void mySensorStatusCheckHandler();

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

void setup()
{
  Serial.begin(9600);
  pinMode(EXINTERRUPT_PIN, INPUT_PULLUP);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  Serial.println("External Interrupt Test using PIR Sensor");
  attachInterrupt(EXINTERRUPT_PIN, onInterruptTriggerChangeHandler, CHANGE);
}
void loop()
{
  mySensorStatusCheckHandler();
}

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
        flgExternalInterruptDetected = false;
      }
    }
    SensorChkTimePreviousMillis = millis();
  }
}
