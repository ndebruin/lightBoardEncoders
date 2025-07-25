#include <Arduino.h>


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Wheel.h"


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{

    Wire.setSDA(18);
    Wire.setSCL(19);
    Wire.begin();

    SerialUSB.begin(9600);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, false)){
        SerialUSB.println("display startup failed");
    }


    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.cp437(true);

    display.print("abcdefghijklmnopqrstuvwxyz");

    display.display();
}



void loop()
{

}