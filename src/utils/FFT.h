/*
 * MIT License
 * 
 * Copyright (c) 2023 Mikko Johannes Heinänen 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#ifndef _FFT_H_
#define _FFT_H_

#include <inttypes.h>
#include "../config.h"
#include "debug.h"
#include "arch/avr/atmega328p/timer1.h"
#include "FFT_strategy.h"
#include "../lib/Fixed8FFT/Fixed8FFT.h"


class FFT
{
private:
    FFT_backend_template* fft = nullptr;
    static timer1 timer;

public:
    FFT( uint8_t input_pin, uint16_t sample_size, uint16_t frequency, fft_backend bits = fixed_8 )
    {
        switch( bits ) {
            case fixed_8:
                fft = new Fixed8FFT(input_pin, sample_size, frequency, bits); 
                break;
            
            default:
                ERROR(F("Invalid bits number"));

                #ifdef DEBUG_CHECKS
                if ( bits > fixed_8 ) ERROR(F("Bits number: "), bits, F(" isnt't implemented"));
                #endif
                return;
        }
        if (fft == nullptr)
        {
            ERROR(F("Not enough memory for fft backend: "), bits);
            return;
        }
                
        if (fft->get_read_vector() == nullptr)
        {
            INFO(F("FFT: fft backend doesn't have read isr"));
            free(fft); // maybe i should just let the fft obj handle it | aka not free it
            return;
        }

        /* TODO: Seutup interrupt's for sampling */
        if (get_isr_vector(TIMER1_COMPB_) == fft->get_read_vector())
        {
            ERROR(F("FFT: Timer1 compb vector is already in use. Exiting"));
            free(fft);
            return;
        }

        /* Check if fft objects data ptr is available */
        if (fft->get_read_vector_data_pointer() == nullptr) 
        {
            #ifdef DEBUG_CHECKS
            INFO(F("FFT: Backend: "), bits, F(" doesn't have isr data ptr"));
            #endif
            goto skip_data_ptr_bind;
        }
        
        if (get_isr_data_ptr(TIMER1_COMPB_) != nullptr)
        {
            ERROR(F("FFT: backend data ptr can't be binded. Since someone has already binded pointer to it"));
            #ifdef DEBUG_CHECKS
                Serial.print(F("FFT: Binded data ptr: 0x")); Serial.println((uint16_t) get_isr_data_ptr(TIMER1_COMPB_), HEX);
                Serial.print(F("FFT: Our data ptr: 0x")); Serial.println((uint16_t) fft->get_read_vector_data_pointer(), HEX);
            #endif

            free(fft);
            return;
        }
        /* Everything correct. We can now bind data ptr */
        cli();
        bind_isr_data_ptr(TIMER1_COMPB_, fft->get_read_vector_data_pointer());

        /* Bind isr */
        skip_data_ptr_bind:

        cli();
        bind_isr(TIMER1_COMPB_, fft->get_read_vector());
        fft->m_sampling_frequency = timer.Start(frequency);

        #ifdef DEBUG_CHECKS
            INFO(F("FFT: Target frequency: "), frequency);
            INFO(F("FFT: Achieved frequency: "), fft->m_sampling_frequency);
            Serial.print(F("Function: 0x"));
            Serial.print(reinterpret_cast<long unsigned int>(fft->get_read_vector()) , HEX);
            Serial.println(F(" binded to TIMER1_COMPB interrupt"));
        #endif
        sei();
    }
    
    ~FFT()
    {
        if (fft == nullptr) return;

        cli();
        if (fft->get_read_vector() != nullptr && fft->get_read_vector() == get_isr_vector(TIMER1_COMPB_))
        {
            /* Check if fft objects data ptr is used */
            if (fft->get_read_vector_data_pointer() != nullptr && fft->get_read_vector_data_pointer() == get_isr_data_ptr(TIMER1_COMPB_))
            {
                unbind_isr_data_ptr(TIMER1_COMPB_);
            }

            /* Unbind isr */
            timer.Stop();
            unbind_isr(TIMER1_COMPB_);
        }
        sei();
        free(fft);
    }

    uint16_t calculate()
    {
        if (fft == nullptr)
        {
            ERROR(F("FFT: No FFT backend initialized"));
            return 0;
        }

        return fft->calculate();
    }
};


#endif