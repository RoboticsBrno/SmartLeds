#pragma once

#include <esp_system.h>
#include <stdint.h>

#if defined(ESP_IDF_VERSION)
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
#define SMARTLEDS_NEW_RMT_DRIVER 1
#else
#define SMARTLEDS_NEW_RMT_DRIVER 0
#endif
#else
#define SMARTLEDS_NEW_RMT_DRIVER 0
#endif

namespace detail {

struct TimingParams {
    uint32_t T0H;
    uint32_t T1H;
    uint32_t T0L;
    uint32_t T1L;
    uint32_t TRS;
};

using LedType = TimingParams;

} // namespace detail

#if SMARTLEDS_NEW_RMT_DRIVER
#include "RmtDriver5.h"
#else
#include "RmtDriver4.h"
#endif
