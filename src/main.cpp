#include <Arduino.h>
// #include <OSCBoards.h>
#include <OSCMessage.h>

// holy semi-broken OSC include
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

// #define DEBUG

#define blinkTime 1000              // 1Hz blink rate
#define displayTime 100             // 10Hz update rate
#define lastParamDebounceTime 20   // 20ms debounce time

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
void updateParamsFromChannel();
bool categoryDiffAndNotNull(Category test, Category compare){ return (test != compare) && (test != Category::None); };


void setup()
{
    // this just gives the namespace a pointer to the actual SLIPserial object. 
    // we can't put it above with the other constructors bc it's function code.
    EosComms::initialize(&SLIPSerial, &storage); 

    // start display
    #ifndef STM32
    Wire.setSDA(DISPLAY_SDA);
    Wire.setSCL(DISPLAY_SCL);
    Wire.begin();
    display.begin();
    #endif

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

    // set our initial parameter indices so they're unset
    wheel1.setParameterIndex(-1);
    wheel2.setParameterIndex(-1);
    wheel3.setParameterIndex(-1);
    wheel4.setParameterIndex(-1);

    // start OSC connection
    EosComms::begin();
};

unsigned long blinkTimer; bool blinkState;
unsigned long displayTimer;
void loop()
{
    updateBlink();

    // keep connection to Eos updated
    EosComms::update();

    // if we're connected, keep updating our inputs/outputs
    if(EosComms::isConnected()){
        // update our inputs / various things
        updateWheels();
        updateParamButtons();
        updateParamsFromChannel();

        // only update our display on a limited rate to not bog the MCU down
        if(millis() - displayTimer > displayTime){
            #ifndef STM32
            updateDisplay();
            #endif
            displayTimer = millis();
        };
    }
    // if we disconnect, show our init display
    else{
        display.showDisplayInit();
    }
};

void updateBlink()
{
    if(millis() - blinkTimer > (blinkTime/2)){
        digitalWrite(LED_BUILTIN, blinkState);
        blinkState = !blinkState;
        blinkTimer = millis();
    }
};

// this should really be inlined into Display.h, but we'll deal with that when we re-write it to work with the i2c multiplexer
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
    // if our encoders have updates to send, then send them
    if(wheel1.update()){ EosComms::sendWheelData(&wheel1); };
    if(wheel2.update()){ EosComms::sendWheelData(&wheel2); };
    if(wheel3.update()){ EosComms::sendWheelData(&wheel3); };
    if(wheel4.update()){ EosComms::sendWheelData(&wheel4); };
};

unsigned long lastParamDebounceTimer;
uint16_t lastParamCount = 0;
bool lastParamState;
/*
    How do I explain this??

    Due to the polling structure that we're kinda stuck with, this function exists.
    Due to how we process the OSC inputs from eOS/how the output from eOS is sent,
    we don't get all the parameters from a channel in one loop
    
    this function debounces the parameter count, 
    and then updates the selected parameters that are assigned to each encoder when the parameters changed
*/
void updateParamsFromChannel()
{
    // if our param count changes, reset our timer and early exit
    if(storage.getParamCount() != lastParamCount){
        lastParamCount = storage.getParamCount();
        lastParamDebounceTimer = millis();
        lastParamState = false;

        return;
    }

    // otherwise, if we debounce past our delay timer
    if(!lastParamState && (millis() - lastParamDebounceTimer >= lastParamDebounceTime)) {
        lastParamState = true;

        // reset to category page 0
        categoryPage = 0;
        
        // find our new indices
        wheel1.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+0));
        wheel2.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+1));
        wheel3.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+2));
        wheel4.setParameterIndex(storage.find(currentCategory, (categoryPage*4)+3));

        return;
    }
};

void updateParamButtons()
{
    // check if any of our buttons are pressed
    Category newCategory = Category::None;
    if(btnIntens.pressed()) {newCategory = Category::Intensity;};
    if(btnFocus.pressed())  {newCategory = Category::Focus;};
    if(btnColor.pressed())  {newCategory = Category::Color;};
    if(btnShutter.pressed()){newCategory = Category::Shutter;};
    if(btnImage.pressed())  {newCategory = Category::Image;};
    if(btnForm.pressed())   {newCategory = Category::Form;};

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