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


// Debouncer nextDebouncer(buttonDebounceTime);
// Debouncer lastDebouncer(buttonDebounceTime);
Button btnIntens(BTN_INTENS);
Button btnFocus(BTN_FOCUS);
Button btnColor(BTN_COLOR);
Button btnShutter(BTN_SHUTTER);
Button btnImage(BTN_IMAGE);
Button btnForm(BTN_FORM);


void updateBlink();
void updateDisplay();
// void updateNextLastButtons();


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

    wheel1.setParameterIndex(0);
    wheel2.setParameterIndex(0);
    wheel3.setParameterIndex(0);
    wheel4.setParameterIndex(0);

    // start OSC connection
    // this is blocking until it connects so it should be the last thing in setup()
    EosComms::begin();
}

unsigned long blinkTimer;
unsigned long blinkTime = 1000; // 1Hz blink rate
unsigned long displayTimer;
unsigned long displayTime = 100; // 10Hz update rate


void loop()
{
    // update input devices
    wheel1.update();
    wheel2.update();
    wheel3.update();
    wheel4.update();

    btnIntens.update();
    btnFocus.update();
    btnColor.update();
    btnShutter.update();
    btnImage.update();
    btnForm.update();

    // keep connection to Eos updated
    EosComms::update();

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
        digitalToggle(LED_BUILTIN);
        blinkTimer = millis();
    }
}

void updateDisplay()
{   
    display.clear();

    display.println("Channels: " + String(storage.getChannelSelection()));
    
    for(uint i = 0; i<storage.getParamCount(); i++){
        if(i == wheel1.getParameterIndex()){
            display.println(" " + String(storage.getParam(i).name) + "   " + String(storage.getParam(i).value));    
        }
        else{
            display.println(String(storage.getParam(i).name) + "   " + String(storage.getParam(i).value));
        }
    }


    display.show();
}


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