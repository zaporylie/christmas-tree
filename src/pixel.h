#include <Animation.h>

Adafruit_NeoPixel strip1 = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Animation animation1 = Animation(&strip1);
