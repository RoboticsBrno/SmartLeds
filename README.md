# SmartLeds

Simple & intuitive way to drive various smart LEDs on ESP32.

## Supported LEDs:

- WS2812  (RMT driver)
- WS2812B (RMT driver)
- SK6812  (RMT driver)
- WS2813  (RMT driver)
- APA102  (SPI driver)
- LPD8806  (SPI driver)

All the LEDs are driven by hardware peripherals in order to achieve high
performance.

## Drivers

## RMT driver

- can drive up to 8 strings
- occupies the RMT peripheral

## SPI driver

- can drive up to 2 strings
- occupies the SPI peripherals
- clock at 10 MHz

## Available
[PlatformIO - library 1740 - SmartLeds](https://platformio.org/lib/show/1740/SmartLeds)