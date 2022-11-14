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


#ifndef _COLOR_MATH_H_
#define _COLOR_MATH_H_

#include <Arduino.h>
#include <inttypes.h>
#include <math.h>
#include "../config.h"

extern void fill_solid(struct CHSV* leds, int numToFill, const struct CRGB& hsvColor);


struct moving_average_filter
{
    float _window[2];
    float _sampleTotal;
    uint8_t _curReadIndex;
    float _sampleAvg;
    float calc(float val);
}; 

struct weighted_moving_average_filter
{

    // declare _alpha and _fading as floats
    float _Alpha = 0.05;    // smoothing factor
    //float _fading = 0.0;    // fading factor


    //float _Alpha = 0.4;
    float _result = 0.0;
    //float _fading = 1.0;
    float calc(float analogRead);
};

// Expotentially weighted mowing average

struct EWMA
{
    float _alpha = 0.26;        // Lower value smoother
    float _lastValue = 0;    
    float _deltaTime = 0;

    EWMA(float Alpha = 0.26) : _alpha( Alpha ) {}
    float calc( float sensorValue ) {
        _deltaTime = micros() - _deltaTime;
        
        _deltaTime /= 10000.0;

        if (_deltaTime > 1.0) _deltaTime = 1.0;

        float real_alpha =  _deltaTime / exp(1.0 - _alpha);


        _lastValue = (1.0F - real_alpha) * _lastValue + real_alpha * sensorValue;
        log_time();
        return _lastValue;
    }

    void log_time() {
        _deltaTime = micros();
    }
};


/* template for a constant rate of change
    Initialized with max rate of change +, then the time in ms
    example: changeLimiter<int> limiter( 300, 20 );

*/
template< class T > class constantChangeRater {
private:
    T unit; 
    T changeTime;       // lenght of changeRate going
    T stepSize;         // Stepsize per ms
    uint32_t lastTime = 0;

    T lastValue = 0;
public:
    constantChangeRater( T changeRate, T changeTime );       // max change in ms
    ~constantChangeRater();
    T limit( T value );
};

template< class T> constantChangeRater<T>::constantChangeRater( T changeRate, T changeTime)
: unit( changeRate )
, changeTime( changeTime )
{
    stepSize = changeRate / changeTime;
}

template< class T> T constantChangeRater<T>::limit( T value )
{
    // Value wich tells how many steps of changetime we're going
    T timeSteps = ( millis() - lastTime ) / changeTime;

    if ( timeSteps > 1.0 ) timeSteps = 2;

    // lets calculate how many changeRates we want to go
    T changes = (value - lastValue) / unit;

    if (timeSteps < 1.0 )
    {     
        changes = constrain( changes, -1, 1);
    } else {
        if (changes < 0) changes = -1;
        else changes = 1;
        
        changes *= timeSteps;
    }


    lastValue += unit * changes * timeSteps;
    
    lastTime = millis();
    
    return lastValue;
}

template<class T> constantChangeRater<T>::~constantChangeRater()
{
}
#endif