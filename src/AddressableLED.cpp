#include "AddressableLED.h"

LEDTimingParameters ws2811Timing = { 300, 950, 900, 350, 300000 };
LEDTimingParameters ws2812Timing = { 400, 850, 800, 450, 300000 };
LEDTimingParameters ws2813Timing = { 350, 800, 350, 350, 300000 };
LEDTimingParameters sk6812Timing = { 300, 900, 600, 600, 80000 };

// TODO: copied from APA106. Need to verify
LEDTimingParameters apa102Timing = { 400, 1250, 1250, 400, 50000 };

std::map<LEDType, LEDTimingParameters> AddressableLED::ledTiming = {
  std::make_pair(NeoPixel, ws2812Timing),
  std::make_pair(WS2812, ws2812Timing),
  std::make_pair(WS2813, ws2813Timing),
  std::make_pair(SK6812, sk6812Timing),
  std::make_pair(SK6812_RGBW, sk6812Timing),
  std::make_pair(DotStar, apa102Timing),
  std::make_pair(APA102, apa102Timing)
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
    vPortCPUInitializeMutex(&_transmitMutex);
  }

AddressableLED::~AddressableLED() {
  delete[] _pixels;
  free(_buffer);
}

void AddressableLED::show() {
  // copy into raw uint8_t buffer
  portENTER_CRITICAL(&_transmitMutex);
  for (uint16_t i = 0; i < _count; i++)
    pixelToRaw(&_pixels[i], i);
  portEXIT_CRITICAL(&_transmitMutex);

  startTransmission();
}