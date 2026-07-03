#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "Parameter.h"
#include "Strings.h"

#define DISPLAY_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

class Display
{
    public:
        Display(TwoWire* wireInterface)
            : display(Adafruit_SSD1306((uint8_t)SCREEN_WIDTH, (uint8_t)SCREEN_HEIGHT, wireInterface, -1))
            {};

        bool begin()
        {
            bool status = display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR, true, false);

            if(!status){
                return false;
            }

            display.clearDisplay();

            showDisplayInit();

            return true;
        };

        void update()
        {

        };

        void displayParam(Parameter param)
        {
            setTextSettings();

            // display.println(param.index);
            display.println(param.name);
            // display.println(param.category);
            display.println(param.value);

            display.display();
        }

        void showDisplayInit()
        {
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.cp437(true);

            display.println(initString1);
            display.println(initString2);
            display.println(initString3);
            display.println(initString4);


            display.display();
        };

        void clear()
        {
            display.clearDisplay();
            display.setCursor(0,0);
        };

        void println(String input){
            setTextSettings();

            display.println(input);
        };

        void println(const char* str)
        {
            setTextSettings();

            display.println(str);
        }

        void show()
        {
            display.display();
        }

    private:
        Adafruit_SSD1306 display;

        void setTextSettings()
        {
            display.setTextSize(2);
            display.setTextColor(SSD1306_WHITE);
            display.cp437(true);
        };

};