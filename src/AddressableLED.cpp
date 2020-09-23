#include "AddressableLED.h"

std::string AddressableLED::TAG = "AddressableLED";

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