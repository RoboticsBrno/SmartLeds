#pragma once

#include <memory>
#include <cassert>
#include <cstring>
#include <stdio.h>

#include <freertos/FreeRTOS.h>

#include "Color.h"

enum WireType { OneWire, TwoWire };
enum PixelOrder { RGB, GRB, RGBW, GRBW };

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