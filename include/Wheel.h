#pragma once

#include <Arduino.h>
#include <Encoder.h>
#include <string.h>

#include "Util.h"
#include "Debouncer.h"

class Wheel
{
    public:
        Wheel(uint8_t encA, uint8_t encB, uint8_t button)
        : encoder(Encoder(encA, encB)), buttonPin(button), debouncer(encoderButtonDebounceTime) {}

        void begin()
        {
            pinMode(buttonPin, INPUT_PULLUP);
        }

        void update()
        {
            // update our ticks
            commandTicks = encoder.read() /4;

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
        }

        // includes count reset, should be used for OSC interactions
        float getCommand()
        {
            encoder.readAndReset(); // reset count to 0 so we're only getting the commands since the last read to get command output
            return getRawCommand();
        }
        WheelMode getMode(){return operationMode;};
        // doesn't reset the count, should be used for debugging
        float getRawCommand(){ return (float)commandTicks; };


        Parameter getTarget(){ return target; };

        // OSC & filter subscription / unsubscription is handled elsewhere
        // we just use the wheel class as a storage location
        
        // Use for changing values when a new parameter is assigned
        void setTarget(Parameter Target){target = Target;};
        // Use for updating stored value when we get an update from Eos
        void setValue(float value){target.value = value;};

    private:
        // hardware bits
        Encoder encoder;
        uint8_t buttonPin;
        
        // button debouncer and edge detector
        Debouncer debouncer;
        bool lastDebounceState;

        // eos software bits
        Parameter target;

        WheelMode operationMode;
        int32_t commandTicks;
};