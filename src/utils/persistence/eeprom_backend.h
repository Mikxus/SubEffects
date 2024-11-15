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
#ifndef _EEPROM_STORAGE_BACKEND_H_
#define _EEPROM_STORAGE_BACKEND_H_

#include <EEPROM.h>
#include "storage_strategy.h"

class eeprom_storage_backend : public storage_backend_template
{
public:
    eeprom_storage_backend(uint16_t min, uint16_t max, uint16_t pos)
        : storage_backend_template(min, max, pos)
    {
    }

    ~eeprom_storage_backend() override {}

    void write(uint16_t index, uint8_t input) override
    {
        EEPROM.write(index, input);
    }

    uint8_t read(uint16_t index) override
    {
        return EEPROM.read(index);
    }
};

#endif