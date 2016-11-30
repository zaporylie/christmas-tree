#ifndef Animation_h

  #define Animation_h
  #include <Adafruit_NeoPixel.h>
  #define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

  enum AnimationProgram {
    COLOR,
    RAINBOW,
    RAINBOW_CYCLE,
    AURORA,
    CUSTOM
  };

  class Animation
  {
    public:
      Animation(Adafruit_NeoPixel*);

      void
        init(uint32_t),
        animate(),
        animate(bool),
        setProgram(AnimationProgram),
        setColor(uint32_t),
        setColor(uint32_t*),
        setDefaultFrameByColor(uint32_t),
        setDefaultFrameByIndex(uint16_t, uint32_t);

      String
        toHex(uint32_t);

      AnimationProgram
        getProgram();

      Adafruit_NeoPixel*
        getStrip();

      uint32_t
        getDefaultFrameByIndex(uint16_t),
        toInt(const char*);

      uint8_t
        getColorRed(uint32_t),
        getColorGreen(uint32_t),
        getColorBlue(uint32_t);

    private:
      void
        rainbow(),
        rainbowCycle(),
        aurora(),
        color(),
        color(uint32_t*),
        toDefaultFrame(uint16_t wait);

      bool
        DEBUG = true,
        nbdelay(),
        nbdelay(unsigned long);

      unsigned long
        millis_last = 0;

      AnimationProgram
        defaultProgram;

      uint32_t
        wheel(byte),
        *defaultFrame,
        iteration = 0;

      Adafruit_NeoPixel
        *_strip;

      const int
        FPS = 48,
        FRAME_TIME = 1000 / FPS;
  };

#endif
