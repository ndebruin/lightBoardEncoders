/*******************************************************************************
  Includes
******************************************************************************/
#include <Arduino.h>
#include <Encoder.h>
#include <LiquidCrystalFast.h>
#include <Bounce.h>

// OSC INCLUDES
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#include <string.h>
#ifdef BOARD_HAS_USB_SERIAL
    #include <SLIPEncodedUSBSerial.h>
    SLIPEncodedUSBSerial SLIPSerial(thisBoardsSerialUSB);
#else
    #include <SLIPEncodedSerial.h>
    SLIPEncodedSerial SLIPSerial(Serial);
#endif

void parseOSCMessage(String &msg);
void issueSubscribes();
void parseEnc1Update(OSCMessage &msg, int addressOffset);

/*******************************************************************************
  Local Includes
******************************************************************************/

const String HANDSHAKE_QUERY = "ETCOSC?";
const String HANDSHAKE_REPLY = "OK";

// put all pin defs in a seperate file to simplify hw changes
#include "pinDefs.h"

/*******************************************************************************
  Global Objects & Variables
******************************************************************************/

// create lcd object
LiquidCrystalFast lcd(RS, RW, EN, D4, D5, D6, D7);

// create debounced objects for the type selections
Bounce intensBtn = Bounce(INTENS_BTN, 10);
Bounce focusBtn = Bounce(FOCUS_BTN, 10);
Bounce beamBtn = Bounce(BEAM_BTN, 10);

void setup()
{
  // initialize the LCD
  lcd.begin(16, 2);
  lcd.clear();

  // initialize the USB OSC connection
  SLIPSerial.begin(115200);

  // This is necessary for reconnecting a device because it needs some time
  // for the serial port to open, but meanwhile the handshake message was
  // sent from Eos
  SLIPSerial.beginPacket();
  SLIPSerial.write((const uint8_t *)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
  SLIPSerial.endPacket();

  Serial1.begin(9600);
  Serial1.println("TEST");
}

void loop()
{
  // update bounce counters
  intensBtn.read();
  focusBtn.read();
  beamBtn.read();

  // check for incoming packet from eos

  String curMsg;
  int size;

  size = SLIPSerial.available();
  if(size > 0){
    while(size--){
      curMsg += (char)(SLIPSerial.read());
    }
  }
  if(SLIPSerial.endofPacket()){
    // eos handshake
    if (curMsg.indexOf(HANDSHAKE_QUERY) != -1)
    {
        // handshake string found!
        SLIPSerial.beginPacket();
        SLIPSerial.write((const uint8_t *)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
        SLIPSerial.endPacket();

        issueSubscribes();
    }
    else{
      parseOSCMessage(curMsg);
    }
  }
}

void parseOSCMessage(String &msg)
{
  OSCMessage oscmsg;
  oscmsg.fill((uint8_t *)msg.c_str(), (int)msg.length());

  oscmsg.route(String("/eos/out/active/wheel/*").c_str(), parseEnc1Update);
}

void parseEnc1Update(OSCMessage &msg, int addressOffset){
  lcd.clear();
  lcd.setCursor(0, 0);
  char temp[50];
  msg.getString(0, temp, 50);
  lcd.println(String(temp));
  
  lcd.println(msg.getInt(1));
}


void issueSubscribes()
{
    // Add a filter so we don't get spammed with unwanted OSC messages from Eos
    OSCMessage filter("/eos/filter/add");
    filter.add("/eos/out/param/*");
    filter.add("/eos/out/ping");
    filter.add("/eos/out/active/wheel/*");
    SLIPSerial.beginPacket();
    filter.send(SLIPSerial);
    SLIPSerial.endPacket();
}