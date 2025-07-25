#include <Arduino.h>

#include "Wheel.h"
#include "Display.h"
#include "Util.h"
#include "Debouncer.h"
#include "Pins.h"

#define DEBUG



Display display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, DISPLAY_ADDR);

Wheel wheel1(ENC_A, ENC_B, ENC_SW);

Debouncer nextDebouncer(buttonDebounceTime);
Debouncer lastDebouncer(buttonDebounceTime);

bool nextButtonState;
bool lastButtonState;

void updateNextLastButtons();


void setup()
{
    SerialUSB.begin(9600);

    // start I2C
    Wire.setSDA(DISPLAY_SDA);
    Wire.setSCL(DISPLAY_SCL);
    Wire.begin();

    // start display
    display.begin();

    // start encoder wheels
    wheel1.begin();

    pinMode(LAST_BTN, INPUT_PULLUP);
    pinMode(NEXT_BTN, INPUT_PULLUP);


    #ifndef DEBUG
    delay(5000);
    #endif
}



void loop()
{
    // update input devices
    wheel1.update();
    updateNextLastButtons();


    display.clear();
    display.println(String(wheel1.getRawCommand()));
    display.println(String(wheel1.getMode()));

    // delay(50);
}




void updateNextLastButtons()
{
    nextButtonState = nextDebouncer.update(!digitalRead(NEXT_BTN), millis());
    lastButtonState = lastDebouncer.update(!digitalRead(LAST_BTN), millis());
}