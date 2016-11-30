#include "Animation.h"

Animation::Animation(Adafruit_NeoPixel* strip) {
  _strip = strip;
}

void Animation::init(uint32_t color) {
  _strip->begin();

  if (defaultFrame) free(defaultFrame);
  uint16_t numBytes = _strip->numPixels() * sizeof(uint32_t);
  if((defaultFrame = (uint32_t *)malloc(numBytes))) {
    memset(defaultFrame, 0, numBytes);
    if (DEBUG) {
      Serial.print("Memory allocated: ");
      Serial.println(numBytes);
    }
  }
  else {
    if (DEBUG) {
      Serial.println("Unable to allocate memory");
    }
  }

  _strip->clear();
  setProgram(COLOR);
  setColor(color);
  // setColor(toInt("ff9900"));
  _strip->show();
  animate(true);
}

uint32_t Animation::toInt(const char* array) {
  char r[5] = {0};
  char g[5] = {0};
  char b[5] = {0};

  r[0] = g[0] = b[0] = '0';
  r[1] = g[1] = b[1] = 'X';

  r[2] = array[0];
  r[3] = array[1];

  g[2] = array[2];
  g[3] = array[3];

  b[2] = array[4];
  b[3] = array[5];

  return _strip->Color(strtol(r, NULL, 16), strtol(g, NULL, 16), strtol(b, NULL, 16));
}

String Animation::toHex(uint32_t color) {
  String string = String(color, HEX);
  while (string.length() < 6) {
    string = String("0") + string;
  }
  return string;
}

Adafruit_NeoPixel* Animation::getStrip() {
  return _strip;
}

void Animation::setProgram(AnimationProgram program) {
  defaultProgram = program;
}

AnimationProgram Animation::getProgram() {
  return defaultProgram;
}

void Animation::animate() {
  animate(false);
}

void Animation::animate(bool blocking) {
  do {
    switch (defaultProgram) {
      case RAINBOW:
        rainbow();
        break;

      case RAINBOW_CYCLE:
        rainbowCycle();
        break;

      case AURORA:
        aurora();
        break;

      case COLOR:
        toDefaultFrame(500);
        break;

      default:
        return;
    }
  } while (blocking && iteration > 0);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Animation::wheel(byte wheelPos) {
  wheelPos = 255 - wheelPos;
  if(wheelPos < 85) {
    return _strip->Color(255 - wheelPos * 3, 0, wheelPos * 3);
  }
  if(wheelPos < 170) {
    wheelPos -= 85;
    return _strip->Color(0, wheelPos * 3, 255 - wheelPos * 3);
  }
  wheelPos -= 170;
  return _strip->Color(wheelPos * 3, 255 - wheelPos * 3, 0);
}

void Animation::setColor(uint32_t color) {
  setDefaultFrameByColor(color);
  iteration = 0;
}

void Animation::setColor(uint32_t* frame) {
  for (int i = 0; i < _strip->numPixels(); i++) {
    setDefaultFrameByIndex(i, frame[i]);
  }
  iteration = 0;
}

void Animation::setDefaultFrameByColor(uint32_t color) {
  for (int i = 0; i < _strip->numPixels(); i++) {
    setDefaultFrameByIndex(i, color);
  }
}

void Animation::setDefaultFrameByIndex(uint16_t index, uint32_t color) {
  defaultFrame[index] = color;
}

uint32_t Animation::getDefaultFrameByIndex(uint16_t index) {
  return *(defaultFrame + index);
}

void Animation::color() {
  color(defaultFrame);
}

void Animation::color(uint32_t* frame) {
  for (int i = 0; i < _strip->numPixels(); i++) {
    _strip->setPixelColor(i, frame[i]);
  }
  _strip->show();
  iteration = 0;
}

bool Animation::nbdelay() {
  nbdelay(FRAME_TIME);
}

bool Animation::nbdelay(unsigned long waitTime) {
  if (millis() - millis_last < waitTime) {
    return false;
  }
  iteration++;
  millis_last = millis();
  return true;
}

uint8_t Animation::getColorRed(uint32_t color) {
  return (uint8_t)(color >> 16);
}

uint8_t Animation::getColorGreen(uint32_t color) {
  return (uint8_t)(color >> 8);
}

uint8_t Animation::getColorBlue(uint32_t color) {
  return (uint8_t)(color >> 0);
}

void Animation::toDefaultFrame(uint16_t transitionTime) {
  uint16_t iterations = (uint16_t)(transitionTime / FRAME_TIME);
  if (!nbdelay()) {
    return;
  }
  if (iteration >= iterations) iteration = 0;

  for (uint16_t i = 0; i < _strip->numPixels(); i++) {
    // Get current color for pixel.
    uint32_t c = _strip->getPixelColor(i);

    int r_offset = (int)(getColorRed(defaultFrame[i]) - getColorRed(c)) / (int)(iterations - iteration);
    int g_offset = (int)(getColorGreen(defaultFrame[i]) - getColorGreen(c)) / (int)(iterations - iteration);
    int b_offset = (int)(getColorBlue(defaultFrame[i]) - getColorBlue(c)) / (int)(iterations - iteration);
    _strip->setPixelColor(i, getColorRed(c) + r_offset, getColorGreen(c) + g_offset, getColorBlue(c) + b_offset);
  }
  _strip->show();
}

void Animation::rainbow() {

  if (!nbdelay()) {
    return;
  }
  if (iteration > 256) iteration = 0;

  for (uint16_t i = 0; i < _strip->numPixels(); i++) {
    _strip->setPixelColor(i, wheel((i+iteration) & 255));
  }
  _strip->show();
}

void Animation::rainbowCycle() {

  if (!nbdelay()) {
    return;
  }
  if (iteration > 256 * 5) iteration = 0;

  for (uint16_t i = 0; i < _strip->numPixels(); i++) {
    _strip->setPixelColor(i, wheel(((i * 256 / _strip->numPixels()) + iteration) & 255));
  }
  _strip->show();
}

void Animation::aurora() {

  if (!nbdelay()) {
    return;
  }
  if (iteration > 140) iteration = 0;

  for (uint16_t i = 0; i < _strip->numPixels(); i++) {
    if (iteration <= 70) {
      _strip->setPixelColor(i, wheel(80 + iteration));
    }
    else {
      _strip->setPixelColor(i, wheel(220 - iteration));
    }
  }
  _strip->show();
}
