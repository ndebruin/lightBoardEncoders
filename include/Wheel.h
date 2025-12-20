#pragma once

#include <Arduino.h>
#include <Encoder.h>
#include <string.h>

#include "Util.h"
#include "Debouncer.h"

enum WheelMode
{
    Coarse = 0,
    Fine = 1
};

class Wheel
{
    public:
        Wheel(uint8_t encA, uint8_t encB, uint8_t button)
        : encoder(Encoder(encA, encB)), buttonPin(button), debouncer(encoderButtonDebounceTime) {}

        void begin()
        {
            pinMode(buttonPin, INPUT_PULLUP);
        };

        void update()
        {
            // update our ticks
            commandTicks = encoder.read() /4;

            // Serial1.println(String(commandTicks));

            // Serial1.println(String(getCommand()));

            // coarse / fine logic
            bool debounceState = debouncer.update(!digitalRead(buttonPin), millis());
            // edge detection if it's actually been pressed
            if(debounceState && !lastDebounceState){
                // change the mode between coarse and fine
                if(operationMode == Coarse) { operationMode = Fine; } 
                else                        { operationMode = Coarse; }
            }

            // help with edge detection
            lastDebounceState = debounceState;
        };

        // includes count reset, should be used for OSC interactions
        float getCommand()
        {            
            float val = getRawCommand();
            reset();
            return val;
        };

        void reset(){ encoder.readAndReset();};
        
        WheelMode getMode(){return operationMode;};
        // doesn't reset the count, should be used for debugging
        float getRawCommand(){ return (float)(commandTicks/1.0); };

        // check if we actually have something to send
        bool haveUpdate(){ return abs(commandTicks) > 0; };

        // get the param index in the data storage
        uint32_t getParameterIndex(){ return paramIndex; };

        // set the param index in the data storage
        void setParameterIndex(uint32_t Index){ paramIndex = Index; };

    private:
        // hardware bits
        Encoder encoder;
        uint8_t buttonPin;
        
        // button debouncer and edge detector
        Debouncer debouncer;
        bool lastDebounceState;

        // index of our wheel parameter in the data storage
        uint32_t paramIndex;

        WheelMode operationMode = WheelMode::Coarse;
        int32_t commandTicks;
};