#include "AddressableLED.h"

std::string AddressableLED::TAG = "AddressableLED";

LEDTimingParameters ws2812Timing = { 350, 700, 800, 600, 50000 };
LEDTimingParameters ws2812bTiming = { 400, 850, 850, 400, 50100 };
LEDTimingParameters ws2813Timing = { 350, 800, 350, 350, 300000 };
LEDTimingParameters sk6812Timing = { 300, 600, 900, 600, 80000 };
LEDTimingParameters apa102Timing = { 300, 600, 900, 600, 80000 };

std::map<LEDType, LEDTimingParameters> AddressableLED::ledTiming = {
  std::make_pair(NeoPixel, ws2812Timing),
  std::make_pair(WS2812, ws2812Timing),
  std::make_pair(WS2812B, ws2812bTiming),
  std::make_pair(WS2813, ws2813Timing),
  std::make_pair(SK6812, sk6812Timing),
  std::make_pair(SK6812_RGBW, sk6812Timing)
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
  // convert into raw uint8_t buffer
  for (uint16_t i = 0; i < _count; i++)
    pixelToRaw(&_pixels[i], i);

  startTransmission();
}