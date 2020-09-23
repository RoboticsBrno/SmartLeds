#pragma once

#include <memory>
#include <cassert>
#include <cstring>
#include <stdio.h>
#include <map>

#include <freertos/FreeRTOS.h>

#include "Color.h"

enum WireType { OneWire, TwoWire };
enum PixelOrder { RGB, GRB, BGR, RGBW, GRBW, WBGR };

enum LEDType : uint8_t {
  NeoPixel,
  WS2812,
  WS2812B,
  WS2813,
  SK6812,
  SK6812_RGBW,
  DotStar,
  APA102,
};

struct LEDTimingParameters {
  uint32_t T0H;
  uint32_t T1H;
  uint32_t T0L;
  uint32_t T1L;
  uint32_t TRS;
};

class AddressableLED {
  protected:
    uint16_t _count;
    WireType _wireType;
    PixelOrder _pixelOrder;
    uint8_t _bytesPerPixel;

    Rgb* _pixels;
    uint8_t* _buffer;

    virtual void startTransmission() = 0;
    static std::string TAG;

    static std::map<LEDType, LEDTimingParameters> ledTiming;

    static uint8_t pixelsForPixelOrder(PixelOrder order) {
      switch (order) {
        case RGB:
        case GRB:
        case BGR:
          return 3;
        case RGBW:
        case GRBW:
        case WBGR:
          return 4;
        default:
          return 3;
      }
    }

  public:
    AddressableLED(int count, WireType wireType, PixelOrder pixelOrder, uint8_t bytesPerPixel);
    ~AddressableLED();

    Rgb& operator[](int idx) { return _pixels[idx]; }
    const Rgb& operator[](int idx) const { return _pixels[idx]; }

    virtual void pixelToRaw(Rgb *pixel, uint16_t index) = 0;

    void show();
    int size() const { return _count; }
    virtual bool wait(uint32_t timeout = portMAX_DELAY) = 0;

    WireType getWireType() { return _wireType; }
};