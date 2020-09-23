#include "TwoWireLED.h"

TwoWireLED::TwoWireLED(spi_host_device_t host, uint16_t count, uint8_t clock, uint8_t data, BufferType bufferType) 
  : AddressableLED(count, WireType::TwoWire, bufferType, 4) {

  spi_bus_config_t buscfg;
  memset(&buscfg, 0, sizeof(buscfg));
  buscfg.mosi_io_num = data;
  buscfg.miso_io_num = -1;
  buscfg.sclk_io_num = clock;
  buscfg.quadwp_io_num = -1;
  buscfg.quadhd_io_num = -1;
  buscfg.max_transfer_sz = 65535;

  spi_device_interface_config_t devcfg;
  memset(&devcfg, 0, sizeof(devcfg));
  devcfg.clock_speed_hz = 1000000;
  devcfg.mode = 0;
  devcfg.spics_io_num = -1;
  devcfg.queue_size = TRANS_COUNT;
  devcfg.pre_cb = nullptr;

  auto ret = spi_bus_initialize(host, &buscfg, 1);
  assert(ret == ESP_OK);

  ret=spi_bus_add_device(host, &devcfg, &_spi);
  assert(ret == ESP_OK);

  std::fill_n(_finalFrame, FINAL_FRAME_SIZE, 0xFFFFFFFF);
}

TwoWireLED::~TwoWireLED() {
  // remove spi device
  auto err = spi_bus_remove_device(_spi);
  assert(err == ESP_OK);

  // todo
}

bool TwoWireLED::wait(uint32_t timeout) {
  for (int i = 0; i != _transCount; i++) {
    spi_transaction_t *t;
    auto err = spi_device_get_trans_result(_spi, &t, timeout);
    if (err != ESP_OK) return false;
  }

  return true;
}

void TwoWireLED::startTransmission() {
  for (int i = 0; i != TRANS_COUNT; i++) {
    _transactions[i].cmd = 0;
    _transactions[i].addr = 0;
    _transactions[i].flags = 0;
    _transactions[i].rxlength = 0;
    _transactions[i].rx_buffer = nullptr;
  }

  // Init frame
  _transactions[0].length = 32;
  _transactions[0].tx_buffer = &_initFrame;
  spi_device_queue_trans(_spi, _transactions + 0, portMAX_DELAY);
  
  // Data
  _transactions[1].length = 32 * _count;
  _transactions[1].tx_buffer = _buffer;
  spi_device_queue_trans(_spi, _transactions + 1, portMAX_DELAY);
  _transCount = 2;

  // End frame
  for (int i = 0; i != 1 + _count / 32 / FINAL_FRAME_SIZE; i++) {
    _transactions[2 + i].length = 32 * FINAL_FRAME_SIZE;
    _transactions[2 + i].tx_buffer = _finalFrame;
    spi_device_queue_trans(_spi, _transactions + 2 + i, portMAX_DELAY);
    _transCount++;
  }
}

void TwoWireLED::pixelToRaw(Rgb *pixel, uint16_t index) {
  uint16_t start = index * FINAL_FRAME_SIZE;
  uint8_t white = std::min(pixel->r, std::min(pixel->g, pixel->b));

  // TODO: set buffer based on pixel order
  _buffer[start] = 0xE0 | white;
  _buffer[start + 3] = pixel->b;
  _buffer[start + 2] = pixel->g;
  _buffer[start + 1] = pixel->r;
}