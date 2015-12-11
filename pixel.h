Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

#define CUSTOM_SHOW -1

int currentMode = DEFAULT_SHOW;
int currentLed = DEFAULT_LED;
String currentColor = DEFAULT_COLOR;

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
  String buf = String(color, HEX);
  while (buf.length() < 6) {
    buf = "0" + buf;
  }
  return buf;
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

// Fill the dots one after the other with a color
void colorWipe(uint8_t wait, uint32_t c = 0) {

  if (c == 0) {
    c = hexToInt(currentColor);
  }

  for(uint16_t i=0; i<strip.numPixels(); i++) {
    if (i < PIXEL_OFFSET) {
      continue;
    }
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
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

//Theatre-style crawling lights.
void theaterChase(uint8_t wait, uint32_t c = 0) {

  if (c == 0) {
    c = hexToInt(currentColor);
  }

  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        if (i+q < PIXEL_OFFSET) {
          continue;
        }
        strip.setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        if (i+q < PIXEL_OFFSET) {
          continue;
        }
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (int i=0; i < strip.numPixels(); i=i+3) {
        if (i+q < PIXEL_OFFSET) {
          continue;
        }
        strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip.show();

      delay(wait);

      for (int i=0; i < strip.numPixels(); i=i+3) {
        if (i+q < PIXEL_OFFSET) {
          continue;
        }
        strip.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

void knightRider(uint8_t wait, uint32_t c = 0) {

  if (c == 0) {
    c = hexToInt(currentColor);
  }

  colorWipe(0, 1);

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
  switch(i){
    case 0: colorWipe(50);
            break;
    case 1: blinkLed(1000);
            break;
    case 2: knightRider(50);
            break;
    case 3: theaterChase(50);
            break;
    case 4: rainbow(20);
            break;
    case 5: rainbowCycle(20);
            break;
    case 6: theaterChaseRainbow(50);
            break;
    default: fallback(5);
            break;
  }
}
