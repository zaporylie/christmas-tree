Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define CUSTOM_SHOW -1

const int FPS = 48;
const int FPS_TIME = (int) 1000/FPS;

int currentMode = DEFAULT_SHOW;
int currentLed = DEFAULT_LED;
String currentColor = DEFAULT_COLOR;

uint32_t nextFrame[PIXEL_COUNT];

void fillNextFrame(uint32_t color) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    nextFrame[i] = color;
  }
}

byte splitColor ( uint32_t c, char value ) {
  switch ( value ) {
    case 'r': return (uint8_t)(c >> 16);
    case 'g': return (uint8_t)(c >>  8);
    case 'b': return (uint8_t)(c >>  0);
    default:  return 0;
  }
}

uint32_t hexToInt(String str) {
  char r[5] = {0};
  char g[5] = {0};
  char b[5] = {0};

  r[0] = g[0] = b[0] = '0';
  r[1] = g[1] = b[1] = 'X';

  r[2] = str[0];
  r[3] = str[1];

  g[2] = str[2];
  g[3] = str[3];

  b[2] = str[4];
  b[3] = str[5];

  return strip.Color(strtol(r, NULL, 16), strtol(g, NULL, 16), strtol(b, NULL, 16));
}

String intToHex(uint32_t color) {
  String string = String(color, HEX);
  while (string.length() < 6) {
    string = String("0") + string;
  }
  return string;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void animateChange(uint8_t steps, uint32_t* color) {
  // Interate in "steps" number of steps.
  for (uint8_t step=0; step < steps; step++) {

    // Change value for each pixel in chain.
    for(uint16_t i = PIXEL_OFFSET; i < strip.numPixels(); i++) {

      // Get current color for pixel.
      uint32_t c = strip.getPixelColor(i);

      uint8_t r_offset = (splitColor(color[i], 'r') - splitColor(c, 'r'))/(steps-step);// ((color - c) / steps) * step;
      uint8_t g_offset = (splitColor(color[i], 'g') - splitColor(c, 'g'))/(steps-step);
      uint8_t b_offset = (splitColor(color[i], 'b') - splitColor(c, 'b'))/(steps-step);
      strip.setPixelColor(i, splitColor(c, 'r') + r_offset, splitColor(c, 'g') + g_offset, splitColor(c, 'b') + b_offset);
    }

    // Set new settings to chain.
    strip.show();

    // Delay for one frame.
    delay(FPS_TIME);
  }
}

void animateChange(uint8_t steps, uint32_t& color) {
  // Interate in "steps" number of steps.
  for (uint8_t step=0; step < steps; step++) {

    // Change value for each pixel in chain.
    for(uint16_t i = PIXEL_OFFSET; i < strip.numPixels(); i++) {

      // Get current color for pixel.
      uint32_t c = strip.getPixelColor(i);

      uint8_t r_offset = (splitColor(color, 'r') - splitColor(c, 'r'))/(steps-step);// ((color - c) / steps) * step;
      uint8_t g_offset = (splitColor(color, 'g') - splitColor(c, 'g'))/(steps-step);
      uint8_t b_offset = (splitColor(color, 'b') - splitColor(c, 'b'))/(steps-step);
      strip.setPixelColor(i, splitColor(c, 'r') + r_offset, splitColor(c, 'g') + g_offset, splitColor(c, 'b') + b_offset);
    }

    // Set new settings to chain.
    strip.show();

    // Delay for one frame.
    delay(FPS_TIME);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint8_t t, uint32_t& color) {
  animateChange(t/FPS_TIME, color);
}

void colorWipe(uint8_t t) {
  animateChange(t/FPS_TIME, nextFrame);
}

void colorWipe() {
  animateChange(1000/FPS_TIME, nextFrame);
}

void pulse(uint16_t n, uint32_t c = 0) {

  if (c == 0) {
    c = hexToInt(currentColor);
  }

  if (n - 3 < strip.numPixels()) {
    strip.setPixelColor(n - 3, 0);
  }

  if (n - 2 >= PIXEL_OFFSET) {
    strip.setPixelColor(n - 2, c/3);
  }

  if (n - 1 >= PIXEL_OFFSET) {
    strip.setPixelColor(n - 1, c*2/3);
  }

  strip.setPixelColor(n, c);

  if (n + 1 < strip.numPixels()) {
    strip.setPixelColor(n + 1, c*2/3);
  }

  if (n + 2 < strip.numPixels()) {
    strip.setPixelColor(n + 2, c/3);
  }

  if (n + 3 < strip.numPixels()) {
    strip.setPixelColor(n + 3, 0);
  }

  strip.show();
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      if (i < PIXEL_OFFSET) {
        continue;
      }
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      if (i < PIXEL_OFFSET) {
        continue;
      }
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void aurora(uint8_t wait) {
  uint16_t i, j;

  for(j=80; j<150; j++) {
    for(i=PIXEL_OFFSET; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(j));
    }
    strip.show();
    delay(wait);
  }

  for(j=150; j>80; j--) {
    for(i=PIXEL_OFFSET; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(j));
    }
    strip.show();
    delay(wait);
  }
}

void knightRider(uint8_t wait) {

  uint32_t c = hexToInt(currentColor);

  strip.clear();

  for (int i = PIXEL_OFFSET; i < strip.numPixels(); i++) {
    pulse(i, c);
    delay(wait);
  }
  for (int i = strip.numPixels() - 1; i >= PIXEL_OFFSET; i--) {
    pulse(i, c);
    delay(wait);
  }
}

void blinkLed(uint16_t wait) {
  uint32_t c = strip.getPixelColor(currentLed);
  strip.setPixelColor(currentLed, 0);
  strip.show();
  delay(wait);
  strip.setPixelColor(currentLed, c);
  strip.show();
  delay(wait);
}

void fallback(uint8_t wait) {
  strip.show();
  delay(wait);
}

void startShow(int i) {
  switch (i) {
    case 0: colorWipe();
            break;
    case 2: knightRider(5);
            break;
    case 4: rainbow(20);
            break;
    case 5: rainbowCycle(20);
            break;
    case 7: aurora(100);
            break;
  }
}
