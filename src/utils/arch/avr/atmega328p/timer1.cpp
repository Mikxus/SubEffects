/*
 * MIT License
 *
 * Copyright (c) 2021 Mikko Johannes Heinänen
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
#include "timer1.h"

constexpr uint8_t prescalers[5] PROGMEM = {
    (1 << CS10) | (0 << CS11) | (0 << CS12), // 0
    (0 << CS10) | (1 << CS11) | (0 << CS12), // 8
    (1 << CS10) | (1 << CS11) | (0 << CS12), // 64
    (0 << CS10) | (0 << CS11) | (1 << CS12), // 256
    (1 << CS10) | (0 << CS11) | (1 << CS12)  // 1024
};

void timer1::SetPrescaler(uint8_t bitmask) // Reference: ATmega328p Datasheet          ! TCR1B register should be set to 0 before calling this function
{                                          // https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf
    TCCR1B = bitmask;                      // Page 110, Table 15-6
    return;
}

uint32_t timer1::SetTimerFrequency(uint32_t frequency)
{
    uint32_t OCR1A_value = 16000000 / frequency;
    uint8_t prescaler_offset = 0;
    const uint16_t prescaler_values[] = {1, 8, 64, 256, 1024};

    /* Loop trough all prescalers from lowest to up. */
    /* Sets the first prescaler wich is the closest to target frequency */
    for (uint8_t i = 0; i < 5; i++)
    {
        if (OCR1A_value <= 0xffff)
        {
            prescaler_offset = i;
            break;
        }
        OCR1A_value = 16000000 / (prescaler_values[i] * frequency);
    }

    if (OCR1A_value > 0xffff)
        OCR1A_value = 0xffff;

    /* Clear prescaler bits & set prescalers */
    TCCR1B &= ~((1 << CS10) | (1 << CS11) | (1 << CS12));
    TCCR1B |= pgm_read_word_near(prescalers + prescaler_offset);
    OCR1A = OCR1A_value;
    return 16000000 / prescaler_values[prescaler_offset] / OCR1A_value;
}

/**
 * @brief Setups timer1's parameters
 *
 * @retval uint32_t: returns frequecy achieved in Hz
 */
uint32_t timer1::Start(uint32_t freq)
{
    cli();
    /* Enable timer if disabled. */
    if (PRTIM1)
        PRR &= ~(1 << PRTIM1);

    TCCR1A = 0;             // set entire TCCR1A register to 0
    TCCR1B = 0;             // same for TCCR1B
    TCNT1 = 0;              // initialize counter value to 0
    TCCR1B |= (1 << WGM12); // turn on CTC mode
    freq = SetTimerFrequency(freq);
    TIMSK1 |= (1 << OCIE1B); // enable timer1 compare B interrupt
    sei();

    /* Return the achieved frequency */
    return freq;
}

/**
 * @brief Stops timer1
 *
 */
void timer1::Stop()
{
    PRR |= (1 << PRTIM1);
    return;
}

/**
 * @brief Re-enables stopped timer
 *
 *
 */
inline void timer1::Continue()
{
    PRR &= ~(1 << PRTIM1);
    return;
}

timer1::~timer1()
{
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = 0;
    TIMSK1 &= 0x4;
    return;
}