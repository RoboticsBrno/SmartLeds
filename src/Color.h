#pragma once

#include <cstdint>
#include "esp_attr.h"
#include <stdint.h>

union Hsv;

union Rgb {
  struct __attribute__ ((packed)) {
    uint8_t r, g, b, w;
  };
  uint32_t value;

  Rgb(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t w = 0xFF) : r(r), g(g), b(b), w(0xE0 | w) {}
  Rgb(Hsv c);
  Rgb& operator=(Rgb rgb) { swap( rgb ); return *this; }
  Rgb& operator=(Hsv hsv);
  Rgb operator+(Rgb in) const;
  Rgb& operator+=(Rgb in);
  void swap(Rgb& o) { value = o.value; }
  void linearize() {
    r = (static_cast<int>(r) * static_cast<int>(r)) >> 8;
    g = (static_cast<int>(g) * static_cast<int>(g)) >> 8;
    b = (static_cast<int>(b) * static_cast<int>(b)) >> 8;
    w = (static_cast<int>(w) * static_cast<int>(w)) >> 8;
  }

  uint8_t IRAM_ATTR getGrb(int idx);
  uint8_t IRAM_ATTR getRgbw(int idx);

  void stretchChannels(uint8_t maxR, uint8_t maxG, uint8_t maxB, uint8_t maxW) {
    r = stretch(r, maxR);
    g = stretch(g, maxG);
    b = stretch(b, maxB);
    w = stretch(w, maxW);
  }

  void stretchChannelsEvenly(uint8_t max) {
    stretchChannels(max, max, max, max);
  }

private:
  uint8_t stretch(int value, uint8_t max) {
    return (value * max) >> 8;
  }
};

union Hsv {
  struct __attribute__ ((packed)) {
    uint8_t h, s, v;
  };
  uint32_t value;

  Hsv(uint8_t h, uint8_t s = 0, uint8_t v = 0) : h(h), s(s), v(v) {}
  Hsv(Rgb r);
  Hsv& operator=(Hsv h) { swap(h); return *this; }
  Hsv& operator=(Rgb rgb);
  void swap(Hsv& o) { value = o.value; }
};
