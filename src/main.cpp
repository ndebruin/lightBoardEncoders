#include <Arduino.h>

// holy OSC includes
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#ifdef BOARD_HAS_USB_SERIAL
#include <SLIPEncodedUSBSerial.h>
SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
#include <SLIPEncodedSerial.h>
SLIPEncodedSerial SLIPSerial(Serial);
#endif
#include <string.h>

#include "Wheel.h"
#include "Display.h"
#include "Util.h"
#include "Debouncer.h"
#include "Pins.h"
#include "EosComms.h"
#include "DataStorage.h"

#define DEBUG

DataStorage storage;

Display display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, DISPLAY_ADDR);

Wheel wheel1(ENC_A, ENC_B, ENC_SW);

Debouncer nextDebouncer(buttonDebounceTime);
Debouncer lastDebouncer(buttonDebounceTime);

bool nextButtonState;
bool lastButtonState;

void updateNextLastButtons();


void setup()
{
    // this just gives the namespace a pointer to the actual SLIPserial object. 
    // we can't put it above with the other constructors bc it's function code.
    EosComms::initialize(&SLIPSerial, &storage); 
    
    // SerialUSB.begin(9600);

    // start I2C for display
    Wire.setSDA(DISPLAY_SDA);
    Wire.setSCL(DISPLAY_SCL);
    Wire.begin();

    // start display
    display.begin();

    // start encoder wheels
    wheel1.begin();

    // start buttons
    pinMode(LAST_BTN, INPUT_PULLUP);
    pinMode(NEXT_BTN, INPUT_PULLUP);

    pinMode(LED_BUILTIN, OUTPUT);

    // start OSC connection
    // this is blocking until it connects so it should be the last thing in setup()
    EosComms::begin();
}

unsigned long blinkTimer;


void loop()
{
    // update input devices
    wheel1.update();
    updateNextLastButtons();

    // keep connection to Eos updated
    EosComms::update();

    // if our encoders have updates to send, then send them
    if(wheel1.haveUpdate()){
        EosComms::sendWheelData(&wheel1);
    }

    if(EosComms::isConnected()){
        display.clear();
        
        display.println(String(EosComms::getTimeSinceRX()));
        display.println(EosComms::getLastRXMessage());
        // if(storage.getParamCount() > 0){
        //     display.displayParam(storage.getParam(0));
        // }
    }

    // display.clear();
    // display.println(String(wheel1.getRawCommand()));
    // display.println(String(wheel1.getMode()));
    
    // delay(50);
    // if(millis() - blinkTimer > 1000){
    //     digitalToggle(LED_BUILTIN);
    //     blinkTimer = millis();
    // }

    if(EosComms::getTimeSinceRX() > 1000){
        digitalWrite(LED_BUILTIN, LOW);
    }
}




void updateNextLastButtons()
{
    nextButtonState = nextDebouncer.update(!digitalRead(NEXT_BTN), millis());
    lastButtonState = lastDebouncer.update(!digitalRead(LAST_BTN), millis());
}