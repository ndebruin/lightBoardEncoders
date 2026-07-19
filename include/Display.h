#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#include "Parameter.h"
#include "Strings.h"

#define DISPLAY_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

class Display
{
    public:
        Display(TwoWire* wireInterface)
            : _display(Adafruit_SSD1306((uint8_t)SCREEN_WIDTH, (uint8_t)SCREEN_HEIGHT, wireInterface, -1))
            {};

        bool begin()
        {
            bool status = _display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR, true, false);

            if(!status){
                return false;
            }

            _display.clearDisplay();

            showDisplayInit();

            return true;
        };

        void update()
        {

        };

        void displayParam(Parameter param)
        {
            setTextSettings();

            _display.print(String(param.index) + " ");
            _display.print(param.name);
            _display.print("    ");
            _display.print(String(param.category) + " ");
            _display.println(param.value);

            // _display.display();
        }

        void showDisplayInit()
        {
            _display.setTextSize(1);
            _display.setTextColor(SSD1306_WHITE);
            _display.cp437(true);

            _display.println(initString1);
            _display.println(initString2);
            _display.println(initString3);
            _display.println(initString4);


            _display.display();
        };

        void clear()
        {
            _display.clearDisplay();
            _display.setCursor(0,0);
        };

        void println(String input){
            setTextSettings();

            _display.println(input);
        };

        void println(const char* str)
        {
            setTextSettings();

            _display.println(str);
        }

        void show()
        {
            _display.display();
        }

    private:
        Adafruit_SSD1306 _display;

        void setTextSettings()
        {
            _display.setTextSize(1);
            _display.setTextColor(SSD1306_WHITE);
            _display.cp437(true);
        };

};