#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <arduino-timer.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 myOledDisplay(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Timer<1, micros> timer;
unsigned long myTimeChk;
unsigned long myRandNumber;

bool myTaskScheduler(void *)
{
  myTimeChk = millis();
  Serial.println(myTimeChk);
  myRandNumber = random(65535);
  myOledDisplay.fillRect(0, 0, 128, 8, BLACK);
  myOledDisplay.setCursor(0, 0);
  myOledDisplay.print(myTimeChk);
  myOledDisplay.print(" ");
  myOledDisplay.print(myRandNumber);
  myTimeChk = millis();
  Serial.println(myTimeChk);
  myRandNumber = random(65535);
  myOledDisplay.fillRect(0, 8, 128, 8, BLACK);
  myOledDisplay.setCursor(0, 8);
  myOledDisplay.print(myTimeChk);
  myOledDisplay.print(" ");
  myOledDisplay.print(myRandNumber);
  myTimeChk = millis();
  Serial.println(myTimeChk);
  myRandNumber = random(65535);
  myOledDisplay.fillRect(0, 16, 128, 8, BLACK);
  myOledDisplay.setCursor(0, 16);
  myOledDisplay.print(myTimeChk);
  myOledDisplay.print(" ");
  myOledDisplay.print(myRandNumber);
  myOledDisplay.display();
  return 1;
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  delay(100);
  if (!myOledDisplay.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  timer.every(1000000, myTaskScheduler); // Every 1000ms -> 1s
  myOledDisplay.clearDisplay();
  myOledDisplay.setTextSize(1);
  myOledDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  myOledDisplay.fillRect(0, 0, 128, 8, BLACK);
  myOledDisplay.setCursor(0, 0);
  myOledDisplay.print("Hello");
  myOledDisplay.fillRect(0, 8, 128, 8, BLACK);
  myOledDisplay.setCursor(0, 8);
  myOledDisplay.print("OLED Control");
  myOledDisplay.fillRect(0, 16, 128, 8, BLACK);
  myOledDisplay.setCursor(0, 16);
  myOledDisplay.print("Edge Device");
  myOledDisplay.setCursor(0, 24);
  myOledDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  myOledDisplay.print(" 2024 IoT MAKEATHON ");
  myOledDisplay.display();
  delay(2000);
  myOledDisplay.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}
void loop()
{
  timer.tick();
}
