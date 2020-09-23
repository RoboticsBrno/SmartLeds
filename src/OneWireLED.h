#pragma once
#include "AddressableLED.h"

#include <esp_intr_alloc.h>
#include <driver/gpio.h>
#include <freertos/semphr.h>
#include <soc/gpio_sig_map.h>
#include <soc/dport_reg.h>
#include <soc/rmt_struct.h>
#include <driver/rmt.h>
#include "esp_log.h"

struct LedTypeParameters {
  uint32_t T0H;
  uint32_t T1H;
  uint32_t T0L;
  uint32_t T1L;
  uint32_t TRS;
  uint8_t bytesPerPixel;
  PixelOrder pixelOrder;
};

using LedType = LedTypeParameters;

static const LedType LED_WS2812         = { 350, 700, 800, 600, 50000, 3, GRB };
static const LedType LED_WS2812B        = { 400, 850, 850, 400, 50100, 3, GRB };
static const LedType LED_SK6812         = { 300, 600, 900, 600, 80000, 3, GRB };
static const LedType LED_SK6812RGBW     = { 300, 600, 900, 600, 80000, 4, RGBW };
static const LedType LED_WS2813         = { 350, 800, 350, 350, 300000, 3, GRB };

struct Timing {
  rmt_item32_t bit0;
  rmt_item32_t bit1;
  uint32_t reset;
  uint8_t bytesPerPixel;
};

union RmtPulsePair {
  struct {
    int duration0:15;
    int level0:1;
    int duration1:15;
    int level1:1;
  };
  uint32_t value;
};

class OneWireLED : public AddressableLED {
  LedType _ledParameters;
  uint8_t _pin;
  rmt_channel_t _channel;

  RmtPulsePair _bitToRmt[2];
  Timing _timing;
  intr_handle_t _interruptHandle;

  uint16_t _pixelPosition;
  uint16_t _componentPosition;
  uint16_t _halfIdx;

  SemaphoreHandle_t _finishedFlag;

  static constexpr int DIVIDER = 4; // 8 still seems to work, but timings become marginal
  static constexpr int MAX_PULSES = 32; // A channel has a 64 "pulse" buffer - we use half per pass
  static constexpr double RMT_DURATION_NS = 12.5; // minimum time of a single RMT duration based on clock ns

  static void initChannel(uint8_t channel);
  static bool anyAlive();
  static OneWireLED*& IRAM_ATTR ledForChannel(rmt_channel_t channel);
  static void IRAM_ATTR interruptHandler(void *);
  void IRAM_ATTR copyRmtHalfBlock();
  void startTransmission();
  void pixelToRaw(Rgb *pixel, uint16_t index);

  static void IRAM_ATTR translateToRMT(const void *src, rmt_item32_t *dest, 
    size_t src_size, size_t wanted_num, size_t *translated_size, size_t *item_num, void* context);

  public:
    OneWireLED(LedType type, uint8_t pin, uint8_t channel, uint16_t count);
    ~OneWireLED();

    bool wait(uint32_t timeout = portMAX_DELAY);

    Rgb getPixel(uint16_t index);
    void setPixel(uint16_t index, Rgb pixel);
};