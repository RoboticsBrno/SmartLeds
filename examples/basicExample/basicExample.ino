#include <Arduino.h>
#include <AddressableLED.h>
#include <OneWireLED.h>
#include <TwoWireLED.h>

const int LED_COUNT = 15;
const int DATA_PIN = 22;
const int CHANNEL = 0;
const int CLK_PIN = 23;


AddressableLED *leds;

// SmartLed -> RMT driver (WS2812/WS2812B/SK6812/WS2813)
leds = new OneWireLED(WS2812B, CHANNEL, DATA_PIN, LED_COUNT);

// APA102 -> SPI driver
// leds = new TwoWireLED(HSPI_HOST, LED_COUNT, CLK_PIN, DATA_PIN)

void setup() {
  Serial.begin(9600);  
}

uint8_t hue;
void showGradient() {
  hue++;
  // Use HSV to create nice gradient
  for ( int i = 0; i != LED_COUNT; i++ )
    (*leds)[i] = Hsv{ static_cast<uint8_t>(hue + 30 * i), 255, 255 };
  leds->show();
  // Show is asynchronous; if we need to wait for the end of transmission,
  // we can use leds.wait(); however we use double buffered mode, so we
  // can start drawing right after showing.
}

void showRgb() {
  (*leds)[0] = Rgb{ 255, 0, 0 };
  (*leds)[1] = Rgb{ 0, 255, 0 };
  (*leds)[2] = Rgb{ 0, 0, 255 };
  (*leds)[3] = Rgb{ 0, 0, 0 };
  (*leds)[4] = Rgb{ 255, 255, 255 };
  leds->show();
}

void loop() {
  Serial.println("New loop");
  
  if (millis() % 10000 < 5000)
    showGradient();
  else
    showRgb();
  delay(50);
}
