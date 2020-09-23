# AddressableLED

A fork of [SmartLeds](https://github.com/RoboticsBrno/SmartLeds) that gives lets you drive RGB and RGBW addressable LEDs via RMT and SPI.

## Differences from SmartLeds

* Supports both RGB and RGBW type of leds
* Gives you the option to specify pixel order (useful for chipsets that operate differently from the standard GRB order)
* Utilizes the built-in ESP32 RMT driver to translate pixel data into RMT signals.
* Renames `SmartLed` to `OneWireLED` and `Apa102` to `TwoWireLED`. Both inherit from an abstract class called `AddressableLED`

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