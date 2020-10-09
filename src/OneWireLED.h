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

struct Timing {
  rmt_item32_t bit0;
  rmt_item32_t bit1;
  uint32_t reset;
};

class OneWireLED : public AddressableLED {
  LEDType _type;
  uint8_t _pin;
  rmt_channel_t _channel;
  LEDTimingParameters _ledParameters;

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
    OneWireLED(LEDType type, uint8_t pin, uint8_t channel, uint16_t count, PixelOrder pixelOrder = PixelOrder::GRB);
    ~OneWireLED();

    bool wait(uint32_t timeout = portMAX_DELAY);
};