#define MAXIMUM_NUM_NEOPIXELS 5

#include <NeoPixelConnect.h>

NeoPixelConnect p(20, MAXIMUM_NUM_NEOPIXELS, pio0, 0);

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("In setup");
}

void loop()
{

  p.neoPixelFill(255, 0, 0, true);
  delay(1000);
  p.neoPixelClear(true);
  delay(1000);

  p.neoPixelFill(0, 255, 0, true);
  delay(1000);
  p.neoPixelClear(true);
  delay(1000);

  p.neoPixelFill(0, 0, 255, true);
  delay(1000);
  p.neoPixelClear(true);
  delay(1000);
}
