#pragma once

#include <Arduino.h>
#include <string.h>

#if defined(TEENSYDUINO)
#include <Encoder.h>
#elif defined(STM32)
#include "Encoder32.h"
#endif

#include "Parameter.h"
#include "Button.h"

enum WheelMode
{
    Coarse = 0,
    Fine = 1
};

class Wheel
{
    public:
    #ifdef STM32
        Wheel(uint8_t button)
        : _encoder(), _btn(button) {}
    #else
        Wheel(uint8_t encA, uint8_t encB, uint8_t button)
        : _encoder(Encoder(encA, encB)), _btn(button) {}
    #endif

    #ifdef STM32
        void begin(TIM_HandleTypeDef* htim)
        {
            _encoder.begin(htim);
            begin();
        }
    #endif

        void begin()
        {
            _btn.begin();
        };

        void update()
        {
            // update our ticks
            _commandTicks = _encoder.read() /4;

            // coarse / fine logic
            if(_btn.pressed()){
                if(_operationMode == Coarse) { _operationMode = Fine; } 
                else                         { _operationMode = Coarse; } 
            }

            // bool debounceState = _debouncer.update(!digitalRead(buttonPin), millis());
            // // edge detection if it's actually been pressed
            // if(debounceState && !lastDebounceState){
            //     // change the mode between coarse and fine
            //     if(operationMode == Coarse) { operationMode = Fine; } 
            //     else                        { operationMode = Coarse; }
            // }

            // // help with edge detection
            // lastDebounceState = debounceState;
        };

        // includes count reset, should be used for OSC interactions
        float getCommand()
        {            
            float val = getRawCommand();
            reset();
            return val;
        };

        void reset(){ _encoder.readAndReset();};
        
        WheelMode getMode(){return _operationMode;};
        // doesn't reset the count, should be used for debugging
        float getRawCommand(){ return (float)(_commandTicks/1.0); };

        // check if we actually have something to send
        bool haveUpdate(){ return abs(_commandTicks) > 0; };

        // get the param index in the data storage
        int32_t getParameterIndex(){ return _paramIndex; };

        // set the param index in the data storage
        void setParameterIndex(int32_t Index){ _paramIndex = Index; };

    private:
        // encoder
        Encoder _encoder;
        
        // button
        Button _btn;

        // index of our wheel parameter in the data storage
        int32_t _paramIndex;

        WheelMode _operationMode = WheelMode::Coarse;
        int32_t _commandTicks;
};