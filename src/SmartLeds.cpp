#include "SmartLeds.h"

IsrCore SmartLed::_interruptCore = CoreCurrent;

SmartLed*& IRAM_ATTR SmartLed::ledForChannel(int channel) {
    static SmartLed* table[detail::CHANNEL_COUNT] = {};
    assert(channel < detail::CHANNEL_COUNT);
    return table[channel];
}
