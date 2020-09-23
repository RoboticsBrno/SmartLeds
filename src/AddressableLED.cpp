#include "AddressableLED.h"

std::string AddressableLED::TAG = "AddressableLED";

std::map<LEDType, LEDTimingParameters> AddressableLED::ledTiming = {
  std::make_pair(NeoPixel, { 350, 700, 800, 600, 50000, 3 });
  std::make_pair(WS2812, { 350, 700, 800, 600, 50000, 3 });
  std::make_pair(WS2812B, { 400, 850, 850, 400, 50100, 3 });
  std::make_pair(WS2813, { 350, 800, 350, 350, 300000, 3 });
  std::make_pair(SK6812, { 300, 600, 900, 600, 80000, 3 });
  std::make_pair(SK2812_RGBW, { 300, 600, 900, 600, 80000, 4 });
};

AddressableLED::AddressableLED(int count, WireType wireType, PixelOrder pixelOrder, uint8_t bytesPerPixel) : 
  _count(count),
  _wireType(wireType),
  _pixelOrder(pixelOrder),
  _bytesPerPixel(bytesPerPixel),
  _pixels(new Rgb[count]),
  _buffer(static_cast<uint8_t*>(malloc(_count * bytesPerPixel)))
  { 
    memset(_buffer, 0x00, _count * _bytesPerPixel);
  }

AddressableLED::~AddressableLED() {
  delete[] _pixels;
  free(_buffer);
}

void AddressableLED::show() {
  // copy into raw uint8_t buffer
  for (uint16_t i = 0; i < _count; i++)
    pixelToRaw(&_pixels[i], i);

  startTransmission();
}