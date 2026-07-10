#include <Arduino.h>

#include <OSCBoards.h>
#include <OSCMessage.h>
#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif


#include "Wheel.h"
#include "Display.h"
#include "Parameter.h"
#include "Debouncer.h"
#include "Pins.h"
#include "EosComms.h"
#include "DataStorage.h"

#define DEBUG

DataStorage storage;

Display display(&Wire);

Wheel wheel1(ENC1_A, ENC1_B, ENC1_SW);
Wheel wheel2(ENC2_A, ENC2_B, ENC2_SW);
Wheel wheel3(ENC3_A, ENC3_B, ENC3_SW);
Wheel wheel4(ENC4_A, ENC4_B, ENC4_SW);


Category currentCategory = Category::None;
uint categoryPage = 0;

Button btnIntens(BTN_INTENS);
Button btnFocus(BTN_FOCUS);
Button btnColor(BTN_COLOR);
Button btnShutter(BTN_SHUTTER);
Button btnImage(BTN_IMAGE);
Button btnForm(BTN_FORM);

void updateBlink();
void updateDisplay();
void updateWheels();
void updateParamButtons();


void setup()
{
    // this just gives the namespace a pointer to the actual SLIPserial object. 
    // we can't put it above with the other constructors bc it's function code.
    EosComms::initialize(&SLIPSerial, &storage); 
    
    // SerialUSB.begin(9600);

    Serial1.begin(115200);

    // start I2C for display
    Wire.setSDA(DISPLAY_SDA);
    Wire.setSCL(DISPLAY_SCL);
    Wire.begin();

    // start display
    display.begin();

    // start encoder wheels
    wheel1.begin();
    wheel2.begin();
    wheel3.begin();
    wheel4.begin();

    // start category buttons
    btnIntens.begin();
    btnFocus.begin();
    btnColor.begin();
    btnShutter.begin();
    btnImage.begin();
    btnForm.begin();

    pinMode(LED_BUILTIN, OUTPUT);

    wheel1.setParameterIndex(-1);
    wheel2.setParameterIndex(-1);
    wheel3.setParameterIndex(-1);
    wheel4.setParameterIndex(-1);

    // start OSC connection
    // this is blocking until it connects so it should be the last thing in setup()
    EosComms::begin();
}


unsigned long blinkTimer; bool blinkState;
unsigned long blinkTime = 1000; // 1Hz blink rate
unsigned long displayTimer;
unsigned long displayTime = 100; // 10Hz update rate

void loop()
{
    // keep connection to Eos updated
    EosComms::update();

    // update input devices
    updateWheels();
    updateParamButtons();

    // if we're connected, keep blinking and updating our display
    if(EosComms::isConnected()){
        updateBlink();
        if(millis() - displayTimer > displayTime){
            updateDisplay();
            displayTimer = millis();
        };
    }


}

void updateBlink()
{
    if(millis() - blinkTimer > (blinkTime/2)){
        digitalWrite(LED_BUILTIN, blinkState);
        blinkState = !blinkState;
        blinkTimer = millis();
    }
}

void updateDisplay()
{   
    display.clear();

    display.println("Channels: " + String(storage.getChannelSelection()) + " Page: " + String(categoryPage));
    
    display.displayParam(storage.getParam(wheel1.getParameterIndex()));
    display.displayParam(storage.getParam(wheel2.getParameterIndex()));
    display.displayParam(storage.getParam(wheel3.getParameterIndex()));
    display.displayParam(storage.getParam(wheel4.getParameterIndex()));

    display.println(String(currentCategory));
    // display.println(String(btnIntens.getState()) + " " + String(btnColor.getState()) + " " + String(btnFocus.getState()) + " " + String(btnImage.getState()) + " " + String(btnShutter.getState()) + " " + String(btnForm.getState()));

    display.show();
};

void updateWheels()
{
    wheel1.update();
    wheel2.update();
    wheel3.update();
    wheel4.update();

    // if our encoders have updates to send, then send them
    if(wheel1.haveUpdate()){
        EosComms::sendWheelData(&wheel1);
    }
    if(wheel2.haveUpdate()){
        EosComms::sendWheelData(&wheel2);
    }
    if(wheel3.haveUpdate()){
        EosComms::sendWheelData(&wheel3);
    }
    if(wheel4.haveUpdate()){
        EosComms::sendWheelData(&wheel4);
    }
};

void updateParamButtons()
{
    // update our debouncers
    // btnIntens.update();
    // btnFocus.update();
    // btnColor.update();
    // btnShutter.update();
    // btnImage.update();
    // btnForm.update();

    // check if any of our buttons are pressed
    Category newCategory = Category::None;
    if(btnIntens.update()){newCategory = Category::Intensity;};
    if(btnFocus.update()){newCategory = Category::Focus;};
    if(btnColor.update()){newCategory = Category::Color;};
    if(btnShutter.update()){newCategory = Category::Shutter;};
    if(btnImage.update()){newCategory = Category::Image;};
    if(btnForm.update()){newCategory = Category::Form;};

    if(newCategory == Category::None){return;} // do nothing

    else if(newCategory == currentCategory){ // rotate our selection
        categoryPage++;

        if((4*categoryPage) >= storage.getCategoryParamCount(currentCategory)){
            categoryPage = 0;
        }

        wheel1.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+0));
        wheel2.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+1));
        wheel3.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+2));
        wheel4.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+3));
        return;
    }

    else if(newCategory != currentCategory){ // switch category
        currentCategory = newCategory;
        categoryPage = 0;

        wheel1.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+0));
        wheel2.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+1));
        wheel3.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+2));
        wheel4.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+3));
    }

};


// void updateNextLastButtons()
// {
//     nextButtonState = nextDebouncer.update(!digitalRead(NEXT_BTN), millis());
//     lastButtonState = lastDebouncer.update(!digitalRead(LAST_BTN), millis());

//     // handle our next/last param button logic
//     if(nextButtonState && !nextButtonPressed){
//         nextButtonPressed = true;
//         uint32_t currentIndex = wheel1.getParameterIndex();
//         if(currentIndex < storage.getParamCount()-1){ currentIndex++; };
//         wheel1.setParameterIndex(currentIndex);
//     }
//     if(!nextButtonState){ nextButtonPressed = false; };

//     if(lastButtonState && !lastButtonPressed){
//         lastButtonPressed = true;
//         uint32_t currentIndex = wheel1.getParameterIndex();
//         if(currentIndex > 0){ currentIndex--; };
//         wheel1.setParameterIndex(currentIndex);
//     }
//     if(!lastButtonState){ lastButtonPressed = false; };
// }