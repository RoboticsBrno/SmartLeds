# AddressableLED

A fork of [SmartLeds](https://github.com/RoboticsBrno/SmartLeds) that gives lets you drive RGB and RGBW addressable LEDs via RMT and SPI.

## Supported LEDs:

- WS2812  (RMT driver)
- WS2812B (RMT driver)
- SK6812  (RMT driver)
- WS2813  (RMT driver)
- APA102  (SPI driver)

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