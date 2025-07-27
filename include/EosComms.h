#pragma once

#include <Arduino.h>

// holy OSC includes
#include <OSCBoards.h>
#include <OSCBundle.h>
#include <OSCData.h>
#include <OSCMatch.h>
#include <OSCMessage.h>
#include <OSCTiming.h>
#include <SLIPEncodedUSBSerial.h>
#include <string.h>

#include "Strings.h"
#include "Util.h"
#include "Wheel.h"

class EosComms
{
    public:
        EosComms(SLIPEncodedUSBSerial* slipSerial) : slipSerial(slipSerial){}

        void begin()
        {
            // actually start serial
            slipSerial->begin(115200);

            // This is a hack around an Arduino bug. 
            // It was taken from the OSC library examples
            while (!SerialUSB){};

            // this is necessary for reconnecting a device because it needs some time
            // for the serial port to open, but meanwhile the handshake message was
            // sent from Eos
            sendHandshakeReply();
        };

        void update()
        {
            // check if we've received a new message
            String msg = readMessage();

            // if we actually received something
            if(msg.length() > 0){
                // parse it
                parseOSCMessage(msg);
            }
        };

        // issues a subscribe to Eos for a specific Parameter object
        void Subscribe(Parameter param)
        {
            OSCMessage sub(String("/eos/subscribe/param/"+param.name).c_str());
            sub.add(1);
            sendMessage(sub);
        }

        // issues an unsubscribe to Eos for a specific Parameter object
        void Unsubscribe(Parameter param)
        {
            OSCMessage unsub(String("/eos/subscribe/param/"+param.name).c_str());
            unsub.add(0);
            sendMessage(unsub);
        }

        void sendWheelData(Wheel* wheel)
        {
            OSCMessage msg;
            int index = wheel->getTarget().index;
            if(wheel->getMode() == WheelMode::Fine){
                msg.setAddress(String("/eos/active/wheel/fine/"+index).c_str());
            }
            else{ // coarse - default behavior
                msg.setAddress(String("/eos/active/wheel/coarse/"+index).c_str());
            }
            msg.add(wheel->getCommand());
            sendMessage(msg);
        }

        bool isConnected(){return connected;};


    private:
        SLIPEncodedUSBSerial* slipSerial; // serial object
        
        bool connected = false;
        // integral strings

        unsigned long lastTimeSent;
        unsigned long lastTimeReceived;

        String readMessage()
        {
            String curMsg;
            int size;

            // check if we have gotten any OSC commands/messages from Eos that we need to parse
            size = slipSerial->available();
            if(size > 0){
                // fill the message with all available bytes
                while(size--)
                    curMsg += (char)(slipSerial->read());
            }
            // if we've gotten a whole packet of data
            if(slipSerial->endofPacket()){
                return curMsg;
            }
            // return an empty string if we didn't get a full packet
            return "";
        }

        void parseOSCMessage(String& msg)
        {
            // check to see if this is the handshake string
            if (msg.indexOf(HANDSHAKE_QUERY) != -1){
                // handshake string found!
                sendHandshakeReply();
                return;
            }

            OSCMessage oscmsg;
            oscmsg.fill((uint8_t *)msg.c_str(), (int)msg.length());
        }

        void sendHandshakeReply()
        {
            slipSerial->beginPacket();
            slipSerial->write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
            slipSerial->endPacket();
        }

        void IssueFilters()
        {
            // this tells Eos to only send us specific messages
            OSCMessage filter("/eos/filter/add");
            filter.add("/eos/out/active/chan"); // active channel details
            filter.add("/eos/out/active/wheel/*"); // parameters of the channel
            filter.add("/eos/out/ping"); // ping back messages
            filter.add("/eos/out/param/*"); // updates on parameter values
            sendMessage(filter);
        }

        // sends an OSCMessage object over SLIPSerial
        void sendMessage(OSCMessage& msg)
        {
            slipSerial->beginPacket();
            msg.send(*slipSerial);
            slipSerial->endPacket();
        }
};