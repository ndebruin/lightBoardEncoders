#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Util.h"
#include "Strings.h"

class Display
{
    public:
        Display(uint16_t width, uint16_t height, TwoWire* wireInterface, uint i2cAddress)
            : display(Adafruit_SSD1306((uint8_t)width, (uint8_t)height, wireInterface, -1)),
            addr(i2cAddress)
            {};

        bool begin()
        {
            bool status = display.begin(SSD1306_SWITCHCAPVCC, addr, true, false);

            if(!status){
                return false;
            }

            display.clearDisplay();

            showDisplayInit();

            return true;
        }

        void showDisplayInit()
        {
            setTextSettings();

            display.println(initString1);
            display.println(initString2);
            display.println(initString3);
            display.println(initString4);
            display.println(initString5);
            display.println(initString6);
            display.println(initString7);
            display.println(initString8);


            display.display();
        }

        void clear()
        {
            display.clearDisplay();
            display.setCursor(0,0);
        }

        void println(String input){
            setTextSettings();

            display.println(input);
            display.display();
        }


    private:
        Adafruit_SSD1306 display;
        uint addr;

        void setTextSettings()
        {
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.cp437(true);
        }

};