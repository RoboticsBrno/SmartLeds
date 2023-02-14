#include "SmartLeds.h"

IsrCore SmartLed::_interruptCore = CoreCurrent;

SmartLed*& IRAM_ATTR SmartLed::ledForChannel( int channel ) {
    static SmartLed* table[8] = { nullptr };
    assert( channel < 8 );
    return table[ channel ];
}

void IRAM_ATTR SmartLed::txEndCallback(rmt_channel_t channel, void *arg) {
    xSemaphoreGiveFromISR(ledForChannel(channel)->_finishedFlag, nullptr);
}

