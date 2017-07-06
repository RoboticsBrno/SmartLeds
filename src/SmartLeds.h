#pragma once

/*
 * A C++ driver for the WS2812 LEDs using the RMT peripheral on the ESP32.
 *
 * Jan "yaqwsx" Mrázek <email@honzamrazek.cz>
 *
 * Based on the work by Martin F. Falatic - https://github.com/FozzTexx/ws2812-demo
 */

/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <memory>
#include <cassert>

#if defined ( ARDUINO )
    extern "C" { // ...someone forgot to put in the includes...
        #include "esp32-hal.h"
        #include "esp_intr.h"
        #include "driver/gpio.h"
        #include "driver/periph_ctrl.h"
        #include "freertos/semphr.h"
        #include "soc/rmt_struct.h"
    }
#elif defined ( ESP_PLATFORM )
    extern "C" { // ...someone forgot to put in the includes...
        #include <esp_intr.h>
        #include <driver/gpio.h>
        #include <freertos/FreeRTOS.h>
        #include <freertos/semphr.h>
        #include <soc/dport_reg.h>
        #include <soc/gpio_sig_map.h>
        #include <soc/rmt_struct.h>
    }
    #include <stdio.h>
#endif

#include "Color.h"

namespace detail {

struct TimingParams {
    uint32_t T0H;
    uint32_t T1H;
    uint32_t T0L;
    uint32_t T1L;
    uint32_t TRS;
};

union RmtPulsePair {
    struct {
        int duration0:15;
        int level0:1;
        int duration1:15;
        int level1:1;
    };
    uint32_t value;
};

static const int DIVIDER = 4; // 8 still seems to work, but timings become marginal
static const int MAX_PULSES = 32; // A channel has a 64 "pulse" buffer - we use half per pass
static const double RMT_DURATION_NS = 12.5; // minimum time of a single RMT duration based on clock ns

} // namespace detail

using LedType = detail::TimingParams;

static const LedType LED_WS2812  = { 350, 700, 800, 600, 50000 };
static const LedType LED_WS2812B = { 350, 900, 900, 350, 50000 };
static const LedType LED_SK6812  = { 300, 600, 900, 600, 80000 };
static const LedType LED_WS2813  = { 350, 800, 350, 350, 300000 };

enum BufferType { SingleBuffer = 0, DoubleBuffer };

class SmartLed {
public:
    SmartLed( const LedType& type, int count, int pin, int channel = 0, BufferType doubleBuffer = SingleBuffer )
        : _timing( type ),
          _channel( channel ),
          _count( count ),
          _firstBuffer( new Rgb[ count ] ),
          _secondBuffer( doubleBuffer ? new Rgb[ count ] : nullptr ),
          _finishedFlag( xSemaphoreCreateBinary() )
    {
        assert( channel >= 0 && channel < 8 );

        DPORT_SET_PERI_REG_MASK( DPORT_PERIP_CLK_EN_REG, DPORT_RMT_CLK_EN );
        DPORT_CLEAR_PERI_REG_MASK( DPORT_PERIP_RST_EN_REG, DPORT_RMT_RST );

        PIN_FUNC_SELECT( GPIO_PIN_MUX_REG[ pin ], 2 );
        gpio_matrix_out( static_cast< gpio_num_t >( pin ), RMT_SIG_OUT0_IDX + _channel, 0, 0 );
        gpio_set_direction( static_cast< gpio_num_t >( pin ), GPIO_MODE_OUTPUT );
        initChannel( _channel );

        RMT.tx_lim_ch[ _channel ].limit = detail::MAX_PULSES;
        RMT.int_ena.val |= 1 << ( 24 + _channel );
        RMT.int_ena.val |= 1 << ( 3 * _channel );

        _bitToRmt[ 0 ].level0 = 1;
        _bitToRmt[ 0 ].level1 = 0;
        _bitToRmt[ 0 ].duration0 = _timing.T0H / ( detail::RMT_DURATION_NS * detail::DIVIDER );
        _bitToRmt[ 0 ].duration1 = _timing.T0L / ( detail::RMT_DURATION_NS * detail::DIVIDER );

        _bitToRmt[ 1 ].level0 = 1;
        _bitToRmt[ 1 ].level1 = 0;
        _bitToRmt[ 1 ].duration0 = _timing.T1H / ( detail::RMT_DURATION_NS * detail::DIVIDER );
        _bitToRmt[ 1 ].duration1 = _timing.T1L / ( detail::RMT_DURATION_NS * detail::DIVIDER );

        esp_intr_alloc( ETS_RMT_INTR_SOURCE, 0, interruptHandler, this, &_interruptHandle );
    }

    ~SmartLed() {
        esp_intr_free( _interruptHandle );
        vSemaphoreDelete( _finishedFlag );
    }

    Rgb& operator[]( int idx ) {
        return _firstBuffer[ idx ];
    }

    const Rgb& operator[]( int idx ) const {
        return _firstBuffer[ idx ];
    }

    void show() {
        _buffer = _firstBuffer.get();
        startTransmission();
        swapBuffers();
    }

    void wait() {
        xSemaphoreTake( _finishedFlag, portMAX_DELAY );
    }

private:
    static void initChannel( int channel ) {
        RMT.apb_conf.fifo_mask = 1;  //enable memory access, instead of FIFO mode.
        RMT.apb_conf.mem_tx_wrap_en = 1; //wrap around when hitting end of buffer
        RMT.conf_ch[ channel ].conf0.div_cnt = detail::DIVIDER;
        RMT.conf_ch[ channel ].conf0.mem_size = 1;
        RMT.conf_ch[ channel ].conf0.carrier_en = 0;
        RMT.conf_ch[ channel ].conf0.carrier_out_lv = 1;
        RMT.conf_ch[ channel ].conf0.mem_pd = 0;

        RMT.conf_ch[ channel ].conf1.rx_en = 0;
        RMT.conf_ch[ channel ].conf1.mem_owner = 0;
        RMT.conf_ch[ channel ].conf1.tx_conti_mode = 0;    //loop back mode.
        RMT.conf_ch[ channel ].conf1.ref_always_on = 1;    // use apb clock: 80M
        RMT.conf_ch[ channel ].conf1.idle_out_en = 1;
        RMT.conf_ch[ channel ].conf1.idle_out_lv = 0;
    }

    static void interruptHandler( void *arg ) {
        auto self = reinterpret_cast< SmartLed * >( arg );
        if ( RMT.int_st.val & ( 1 << ( 24 + self->_channel ) ) ) { // tx_thr_event
            self->copyRmtHalfBlock();
            RMT.int_clr.val |= 1 << ( 24 + self->_channel );
        }
        else if ( RMT.int_st.val & ( 1 << ( 3 * self->_channel ) ) ) { // tx_end
            self->endTransmission();
            RMT.int_clr.val |= 1 << ( 3 * self->_channel );
            RMT.int_clr.ch0_tx_end = 1;
        }
    }

    void swapBuffers() {
        if ( _secondBuffer )
            _firstBuffer.swap( _secondBuffer );
    }

    void copyRmtHalfBlock() {
        int offset = detail::MAX_PULSES * _halfIdx;
        _halfIdx = !_halfIdx;
        int len = 3 - _componentPosition + 3 * ( _count - 1 );
        len = std::min( len, detail::MAX_PULSES / 8 );

        if ( !len ) {
            for ( int i = 0; i < detail::MAX_PULSES; i++) {
                RMTMEM.chan[_channel].data32[i + offset].val = 0;
            }
        }

        int i;
        for ( i = 0; i != len && _pixelPosition != _count; i++ ) {
            uint8_t val = _buffer[ _pixelPosition ].getGrb( _componentPosition );
            for ( int j = 0; j != 8; j++, val <<= 1 ) {
                int bit = val >> 7;
                int idx = i * 8 + offset + j;
                RMTMEM.chan[ _channel ].data32[ idx ].val = _bitToRmt[ bit & 0x01 ].value;
            }
            if ( _pixelPosition == _count - 1 && _componentPosition == 2 ) {
                RMTMEM.chan[ _channel ].data32[ i * 8 + offset + 7 ].duration1 =
                    _timing.TRS / ( detail::RMT_DURATION_NS * detail::DIVIDER );
            }

            _componentPosition++;
            if ( _componentPosition == 3 ) {
                _componentPosition = 0;
                _pixelPosition++;
            }
        }

        for ( i *= 8; i != detail::MAX_PULSES; i++ ) {
            RMTMEM.chan[ _channel ].data32[ i + offset ].val = 0;
        }
    }

    void startTransmission() {
        _pixelPosition = _componentPosition = _halfIdx = 0;
        copyRmtHalfBlock();
        if ( _pixelPosition < _count )
            copyRmtHalfBlock();
        xSemaphoreTake( _finishedFlag, 0 );
        RMT.conf_ch[ _channel ].conf1.mem_rd_rst = 1;
        RMT.conf_ch[ _channel ].conf1.tx_start = 1;
    }

    void endTransmission() {
        xSemaphoreGiveFromISR( _finishedFlag, nullptr );
    }

    const LedType& _timing;
    int _channel;
    detail::RmtPulsePair _bitToRmt[ 2 ];
    int _count;
    std::unique_ptr< Rgb[] > _firstBuffer;
    std::unique_ptr< Rgb[] > _secondBuffer;
    Rgb *_buffer;
    intr_handle_t _interruptHandle;

    xSemaphoreHandle _finishedFlag;

    int _pixelPosition;
    int _componentPosition;
    int _halfIdx;
};