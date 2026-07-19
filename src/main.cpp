#include <Arduino.h>
#include <OSCBoards.h>
#include <OSCMessage.h>

#ifdef BOARD_HAS_USB_SERIAL
#ifdef TEENSYDUINO
#include <SLIPEncodedUSBSerial.h>
#else
#include <SLIPEncodedSerial.h>
#endif
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif

#include "Wheel.h"
#include "Display.h"
#include "Parameter.h"  
#include "Button.h"
#include "Pins.h"
#include "EosComms.h"
#include "DataStorage.h"

#define DEBUG

DataStorage storage;

Display display(&Wire);


#ifdef STM32
Wheel wheel1(ENC1_SW);
Wheel wheel2(ENC2_SW);
Wheel wheel3(ENC3_SW);
Wheel wheel4(ENC4_SW);
#else
Wheel wheel1(ENC1_A, ENC1_B, ENC1_SW);
Wheel wheel2(ENC2_A, ENC2_B, ENC2_SW);
Wheel wheel3(ENC3_A, ENC3_B, ENC3_SW);
Wheel wheel4(ENC4_A, ENC4_B, ENC4_SW);
#endif

Category currentCategory = Category::Intensity;
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
void updateFromChannel();
bool categoryDiffAndNotNull(Category test, Category compare);


void setup()
{
    // this just gives the namespace a pointer to the actual SLIPserial object. 
    // we can't put it above with the other constructors bc it's function code.
    EosComms::initialize(&SLIPSerial, &storage); 

    // start display
    Wire.setSDA(DISPLAY_SDA);
    Wire.setSCL(DISPLAY_SCL);
    Wire.begin();
    display.begin();

    

    // start encoder wheels
    #ifdef STM32
    // wheel1.begin(&htim2);
    // wheel2.begin(&htim3);
    // wheel3.begin(&htim21);
    // wheel4.begin(&htim22);
    #else
    wheel1.begin();
    wheel2.begin();
    wheel3.begin();
    wheel4.begin();
    #endif

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
unsigned long lastParamChangeTimer;
unsigned long lastParamDebounceTime = 50; // 20Hz debounce rate
uint16_t lastParamCount = 0;
bool lastParamState;

void loop()
{
    // keep connection to Eos updated
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();
    EosComms::update();

    if(storage.getParamCount() != lastParamCount){
        lastParamCount = storage.getParamCount();
        lastParamChangeTimer = millis();
        lastParamState = false;
    }

    if(!lastParamState && (millis() - lastParamChangeTimer >= lastParamDebounceTime)) {
        lastParamState = true;

        updateFromChannel();
    }
    
    updateWheels();
    updateParamButtons();

    // if we're connected, keep blinking and updating our display
    updateBlink();
    if(EosComms::isConnected()){
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

bool categoryDiffAndNotNull(Category test, Category compare)
{
    return (test != compare) && (test != Category::None);
}


void updateFromChannel()
{
    Category param1 = storage.getParam(wheel1.getParameterIndex()).category;
    Category param2 = storage.getParam(wheel2.getParameterIndex()).category;
    Category param3 = storage.getParam(wheel3.getParameterIndex()).category;
    Category param4 = storage.getParam(wheel4.getParameterIndex()).category;
    if(categoryDiffAndNotNull(param1, currentCategory) || categoryDiffAndNotNull(param2, currentCategory) || categoryDiffAndNotNull(param3, currentCategory) || categoryDiffAndNotNull(param4, currentCategory)
        || (param1 == None && param2 == None && param3 == None && param4 == None)
        ){

        // categoryReset = true;

        // if((4*categoryPage) >= storage.getCategoryParamCount(currentCategory)){
        //     categoryPage = 0;
        // }
        categoryPage = 0;

        wheel1.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+0));
        wheel2.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+1));
        wheel3.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+2));
        wheel4.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+3));


        // if(param1 == Category::None && param2 == Category::None && param3 == Category::None && param4 == Category::None){
        //     // categoryReset = false;
        // }
    }
}

void updateParamButtons()
{
    // check if any of our buttons are pressed
    Category newCategory = Category::None;
    if(btnIntens.pressed()){newCategory = Category::Intensity;};
    if(btnFocus.pressed()){newCategory = Category::Focus;};
    if(btnColor.pressed()){newCategory = Category::Color;};
    if(btnShutter.pressed()){newCategory = Category::Shutter;};
    if(btnImage.pressed()){newCategory = Category::Image;};
    if(btnForm.pressed()){newCategory = Category::Form;};

    // if(newCategory == Category::None){return;} // do nothing

    if(newCategory == Category::None){return;} // do nothing

    else if(newCategory != currentCategory){ // switch category
        currentCategory = newCategory;
        categoryPage = 0;

        wheel1.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+0));
        wheel2.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+1));
        wheel3.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+2));
        wheel4.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+3));
        return;
    }



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