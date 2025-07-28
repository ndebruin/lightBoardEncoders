#include "EosComms.h"

namespace EosComms
{
//////////////////////////////////////////// PRIVATE ////////////////////////////////////////////
// private has to come first bc this is a source file and C is stupid
    namespace { // this creates a functional equivalent of "private:" in a class
        SLIPEncodedUSBSerial* slipSerial; // serial object
        
        bool connected = false;
        // integral strings

        unsigned long lastTimeSent;
        unsigned long lastTimeReceived;

        /// @brief Sends an OSCMessage object over our SLIPSerial object.
        void sendMessage(OSCMessage& msg)
        {
            slipSerial->beginPacket();
            msg.send(*slipSerial);
            slipSerial->endPacket();
            lastTimeSent = millis(); // update our timers
        }

        /// @brief  Sends the handshake reply. Notably not an OSC message.
        void sendHandshakeReply()
        {
            slipSerial->beginPacket();
            slipSerial->write((const uint8_t*)HANDSHAKE_REPLY.c_str(), (size_t)HANDSHAKE_REPLY.length());
            slipSerial->endPacket();
        }

        /// @brief Send message to "/eos/ping". No payload.
        void sendPing()
        {
            OSCMessage ping("/eos/ping");
            sendMessage(ping);
        }

        /// @brief Sends all required filters to Eos in a single OSC Message.
        void IssueFilters()
        {
            // this tells Eos to only send us specific messages
            OSCMessage filter("/eos/filter/add");
            filter.add("/eos/out/active/chan"); // selected channel details
            filter.add("/eos/out/active/wheel/*"); // control parameters of the channel
            filter.add("/eos/out/ping"); // ping back messages
            filter.add("/eos/out/param/*"); // updates on parameter values
            sendMessage(filter);
        }

        /// @brief Handles routing OSC messages to the correct callback functions.
        /// @param msg The OSCMessage object to route.
        void routeOSCMessage(OSCMessage& msg)
        {
            msg.dispatch("/eos/out/ping",handlePingResponse); // ping responses
            msg.dispatch("/eos/out/active/chan",handleChannelUpdate); // selected channel details
            msg.route("/eos/out/active/wheel/*",handleWheelUpdate); // control parameters of the channel
            msg.route("/eos/out/param/*",handleParameterUpdate); // updates on parameter values
        }

        /// @brief Handles reading any bytes in the SLIPSerial buffer into a message.
        /// @return If a full message has been received, returns the message. Returns an empty string otherwise.
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

        /// @brief Handles a received generic string.
        /// @param msg String object that contains a received message.
        void parseMessage(String& msg)
        {
            // check to see if this is the handshake string
            if (msg.indexOf(HANDSHAKE_QUERY) != -1){
                // handshake string found!
                sendHandshakeReply();
                connected = true;
                // also send our filters so that they are given to Eos
                IssueFilters();
                return;
            }

            // if it's not the handshake sequence,
            // pack it into an OSCMessage object
            // and hand it off to our routing function
            OSCMessage oscmsg;
            oscmsg.fill((uint8_t *)msg.c_str(), (int)msg.length());
            routeOSCMessage(oscmsg);
        }
        
    }; // end of empty namespace (equivalent to "private:" for a class)
//////////////////////////////////////////// END OF PRIVATE /////////////////////////////////////////////



//////////////////////////////////////////// Basic Functions ////////////////////////////////////////////

    /// @brief This replaces an equivalent class constructor.
    /// @param SLIPSerial Pointer to a SLIPEncodedUSBSerial object to use for communication to Eos.
    void initialize(SLIPEncodedUSBSerial* SLIPSerial){ slipSerial = SLIPSerial; }

    /// @brief Performs all initialization of our communication with Eos.
    /// @attention Is blocking until a Serial connection is made.
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

    /// @brief Checks for new received messages and handles them.
    void update()
    {
        // check if we've received a new message
        String msg = readMessage();
        // if we actually received something
        if(msg.length() > 0){
            // update our lastReceived timers
            lastTimeReceived = millis();
            // parse it
            parseMessage(msg);
        }

        // timeout logic
        // if we're connected but haven't heard from Eos in a bit, send a keepAlive ping.
        if(connected && (millis() - lastTimeReceived >= timeoutPingTime)){
            sendPing();
        }
        // if we still haven't heard from Eos, then after a while say that we've disconnected.
        if(millis() - lastTimeReceived >= timeoutDisconnectTime){
            connected = false;
        }

    };

//////////////////////////////////////////// Transmission Functions ////////////////////////////////////////////

    /// @brief Issues a subscribe to Eos for a specific Parameter object
    /// @param param Parameter object (name) to subscribe to updates for.
    void Subscribe(Parameter param)
    {
        OSCMessage sub(String("/eos/subscribe/param/"+param.name).c_str());
        sub.add(1);
        sendMessage(sub);
    }

    /// @brief Issues an unsubscribe to Eos for a specific Parameter object
    /// @param param Parameter object (name) to unsubscribe from updates for.
    void Unsubscribe(Parameter param)
    {
        OSCMessage unsub(String("/eos/subscribe/param/"+param.name).c_str());
        unsub.add(0);
        sendMessage(unsub);
    }

    /// @brief Sends command data for a given wheel object when called.
    /// @param wheel The Wheel object you wish to send command data for.
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

//////////////////////////////////////////// Utility Functions ////////////////////////////////////////////

    bool isConnected(){return connected;};

//////////////////////////////////////////// Receive Callbacks ////////////////////////////////////////////

    // handles "/eos/out/ping" messages
    void handlePingResponse(OSCMessage& msg)
    {
    
    }
 
    // handles "/eos/out/active/channel" messages
    void handleChannelUpdate(OSCMessage& msg)
    {
    
    }

    // handles "/eos/out/param/*" messages
    void handleParameterUpdate(OSCMessage& msg, int patternOffset)
    {
    
    }

    // handles "/eos/out/active/wheel/*" messages
    void handleWheelUpdate(OSCMessage& msg, int patternOffset)
    {
    
    }
};