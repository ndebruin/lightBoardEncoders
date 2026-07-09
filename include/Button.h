#pragma once

#include <Arduino.h>

#include "Debouncer.h"

#define buttonDebounceTime 10

class Button
{
    public:
        Button(uint8_t pin)
            : _pin(pin), _debouncer(buttonDebounceTime)
        {};

        void begin()
        {
            pinMode(_pin, INPUT_PULLUP);
        };

        bool update()
        {
            _rawState = _debouncer.update(!digitalRead(_pin), millis());

            // formatted like this rather than a simple assign 
            // bc this handles holding the button
            if(_rawState && !_pressed){
                _pressed = true;
            }
            else if (!_rawState){ 
                _pressed = false;
            };

            return _pressed;
        };

        bool getState()
        {
            return _pressed;
        };

    private:
        uint8_t _pin;

        Debouncer _debouncer;

        bool _pressed;
        bool _rawState;
};