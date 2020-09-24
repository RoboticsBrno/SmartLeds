#include "OneWireLED.h"

uint32_t OneWireLED::_clock = 0;

OneWireLED::OneWireLED(LEDType type, uint8_t pin, uint8_t channel, uint16_t count, PixelOrder pixelOrder) : 
  AddressableLED(count, WireType::OneWire, pixelOrder, pixelsForPixelOrder(pixelOrder)),
  _type(type),
  _pin(pin),
  _channel((rmt_channel_t) channel) {
  assert(channel < 8);
  assert(ledForChannel(_channel) == nullptr);

  // ensure we have timing data for LED type
  auto timing = ledTiming.find(type);
  assert(timing != ledTiming.end());
  _ledParameters = timing->second;

  // configure RMT for GPIO
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX((gpio_num_t) pin, _channel);
  config.clk_div = 2;

  // install RMT driver for channel
  rmt_config(&config);
  rmt_driver_install(_channel, 0, 0);

  auto err = rmt_get_counter_clock(_channel, &_clock);
  if (err != ESP_OK) {
    printf("Unable to get RMT counter clock frequency\n");
    return;
  }

  // install translator
  switch (type) {
    case WS2813:
      err = rmt_translator_init(_channel, translateWS2813);
      break;
    case SK6812:
    case SK6812_RGBW:
      err = rmt_translator_init(_channel, translateSK6812);
    case NeoPixel:
    case WS2812:
    default:
      err = rmt_translator_init(_channel, translateWS2812);
  }
  if (err != ESP_OK) {
    printf("Unable to register RMT translator for channel %d\n", channel);
    return;
  }

  ledForChannel(_channel) = this;
}

OneWireLED::~OneWireLED() {
  // uninstall rmt driver
  rmt_driver_uninstall(_channel);

  ledForChannel(_channel) = nullptr;
  if (!anyAlive())
    esp_intr_free(_interruptHandle);
}

OneWireLED*& IRAM_ATTR OneWireLED::ledForChannel(rmt_channel_t channel) {
  static OneWireLED* table[8] = { nullptr };
  assert(channel < 8);
  return table[channel];
}

bool OneWireLED::wait(uint32_t timeout) {
  return ESP_OK == rmt_wait_tx_done(_channel, timeout / portTICK_PERIOD_MS);
}

void OneWireLED::startTransmission() {
  uint16_t total = _count * _bytesPerPixel;
  auto err = rmt_write_sample(_channel, _buffer, total, true);
  if (err != ESP_OK)
    printf("RMT Transmit failed: %d\n", err);
}

bool OneWireLED::anyAlive() {
  for (int i = 0; i != 8; i++)
    if (ledForChannel((rmt_channel_t)i) != nullptr) return true;
  return false;
}

void IRAM_ATTR OneWireLED::translateWS2812(const void *src, rmt_item32_t *dest, 
  size_t src_size, size_t wanted_num, size_t *translated_size, size_t *item_num) {
    LEDTimingParameters params = ledTiming[WS2812];
    translateToRMT(src, dest, src_size, wanted_num, translated_size, item_num, &params);
}

void IRAM_ATTR OneWireLED::translateWS2813(const void *src, rmt_item32_t *dest, 
  size_t src_size, size_t wanted_num, size_t *translated_size, size_t *item_num) {
    LEDTimingParameters params = ledTiming[WS2813];
    translateToRMT(src, dest, src_size, wanted_num, translated_size, item_num, &params);
}

void IRAM_ATTR OneWireLED::translateSK6812(const void *src, rmt_item32_t *dest, 
  size_t src_size, size_t wanted_num, size_t *translated_size, size_t *item_num) {
    LEDTimingParameters params = ledTiming[SK6812];
    translateToRMT(src, dest, src_size, wanted_num, translated_size, item_num, &params);
}

void IRAM_ATTR OneWireLED::translateToRMT(const void *src, rmt_item32_t *dest, 
  size_t src_size, size_t wanted_num, size_t *translated_size, size_t *item_num, LEDTimingParameters* timingParameters) {

  if (src == NULL || dest == NULL) {
      *translated_size = 0;
      *item_num = 0;
      return;
  }

  size_t size = 0;
  size_t num = 0;
  const uint8_t* psrc = static_cast<const uint8_t*>(src);
  rmt_item32_t* pdest = dest;

  Timing timing;
  // set T0H, T0L, T1H, T1L durations into RMT pulses
  float ratio = (float)_clock / 1e9;
  uint32_t t0HTicks = (uint32_t) (timingParameters->T0H * ratio);
  uint32_t t0LTicks = (uint32_t) (timingParameters->T0L * ratio);
  uint32_t t1HTicks = (uint32_t) (timingParameters->T1H * ratio);
  uint32_t t1LTicks = (uint32_t) (timingParameters->T1L * ratio);
  timing.bit0 = {{{ t0HTicks, 1, t0LTicks, 0 }}};
  timing.bit1 = {{{ t1HTicks, 1, t1LTicks, 0 }}};
  timing.reset = timingParameters->TRS;

  for (;;) {
    uint8_t data = *psrc;
    for (uint8_t bit = 0; bit < 8; bit++)
    {
      pdest->val = (data & 0x80) ? timing.bit1.val : timing.bit0.val;
      pdest++;
      data <<= 1;
    }
    num += 8;
    size++;

    if (size >= src_size || num >= wanted_num)
      break;

    psrc++;
  }
  *translated_size = size;
  *item_num = num;
}

void OneWireLED::pixelToRaw(Rgb *pixel, uint16_t index) {
  auto white = std::min(pixel->r, std::min(pixel->g, pixel->b));
  uint16_t start = index * _bytesPerPixel;

  switch (_pixelOrder) {
    case PixelOrder::RGB:
      _buffer[start] = pixel->r;
      _buffer[start + 1] = pixel->g;
      _buffer[start + 2] = pixel->b;
      break;
    case PixelOrder::BGR:
      _buffer[start] = pixel->b;
      _buffer[start + 1] = pixel->g;
      _buffer[start + 2] = pixel->r;
      break;
    case PixelOrder::RGBW:
      _buffer[start] = pixel->r - white;
      _buffer[start + 1] = pixel->g - white;
      _buffer[start + 2] = pixel->b - white;
      _buffer[start + 3] = white;
      break;
    case PixelOrder::GRBW:
      _buffer[start] = pixel->g - white;
      _buffer[start + 1] = pixel->r - white;
      _buffer[start + 2] = pixel->b - white;
      _buffer[start + 3] = white;
      break;
    case PixelOrder::GRB:
    default:
      _buffer[start] = pixel->g;
      _buffer[start + 1] = pixel->r;
      _buffer[start + 2] = pixel->b;
      break;
  }
}