#pragma once
#include "AddressableLED.h"

#include <driver/spi_master.h>

class TwoWireLED : public AddressableLED {
  static constexpr int FINAL_FRAME_SIZE = 4;
  static constexpr int TRANS_COUNT = 2 + 8;

  spi_device_handle_t _spi;
  spi_transaction_t _transactions[TRANS_COUNT];
  int _transCount;

  void startTransmission();
  void pixelToRaw(Rgb *pixel, uint16_t index);

  uint32_t _initFrame;
  uint32_t _finalFrame[FINAL_FRAME_SIZE];

  public:
    TwoWireLED(spi_host_device_t host, uint16_t count, uint8_t clock, uint8_t data, BufferType buffer = SingleBuffer);
    ~TwoWireLED();

    Rgb& operator[](int idx) { return _firstBuffer[idx]; }
    const Rgb& operator[](int idx) const { return _firstBuffer[idx]; }

    void show();
    bool wait(uint32_t timeout = portMAX_DELAY);
    int size() const;
};