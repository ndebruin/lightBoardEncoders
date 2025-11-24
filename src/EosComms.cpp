#include "EosComms.h"

namespace EosComms
{

    
//////////////////////////////////////////// PRIVATE ////////////////////////////////////////////
// private has to come first bc this is a source file and the C++ preprocessor is stupid
    namespace { // this creates a functional equivalent of "private:" in a class
        SLIPEncodedUSBSerial* slipSerial; // serial object
        DataStorage* storage;
        
        bool connected = false;
        bool sentPing = false;

        unsigned long lastTimeReceived;
        unsigned long lastTimeSent;

        String curMsg;
        String lastMsg;

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
            lastTimeSent = millis();
        }

        /// @brief Send message to "/eos/ping". No payload.
        void sendPing()
        {
            sentPing = true;
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
            // filter.add("/eos/out/param/*"); // updates on parameter values
            sendMessage(filter);
        }

        /// @brief Handles routing OSC messages to the correct callback functions.
        /// @param msg The OSCMessage object to route.
        void routeOSCMessage(OSCMessage& msg)
        {
            
            msg.route("/eos/out/ping",handlePingResponse); // ping responses
            msg.route("/eos/out/active/chan",handleChannelUpdate); // selected channel details
            msg.route("/eos/out/active/wheel/",handleWheelUpdate); // control parameters of the channel
            // msg.route("/eos/out/param/",handleParameterUpdate); // updates on parameter values
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

        /// @brief Handles reading any bytes in the SLIPSerial buffer into a message.
        /// @return If a full message has been received, returns the message. Returns an empty string otherwise.
        void readMessage()
        {
            // String curMsg;
            int size;

            // check if we have gotten any OSC commands/messages from Eos that we need to parse
            size = slipSerial->available();
            if(size > 0){
                // fill the message with all available bytes
                while(size--){
                    curMsg += (char)(slipSerial->read());
                }
            }
            // if we've gotten a whole packet of data, go handle it
            if(slipSerial->endofPacket()){
                // update our lastReceived timers
                lastTimeReceived = millis();
                connected = true;
                sentPing = false;
                lastMsg = curMsg;

                // parse our message
                parseMessage(curMsg);
                curMsg = String();
            }
            

            
        }
        
    }; // end of empty namespace (equivalent to "private:" for a class)
//////////////////////////////////////////// END OF PRIVATE /////////////////////////////////////////////

//////////////////////////////////////////// Basic Functions ////////////////////////////////////////////

    /// @brief This replaces an equivalent class constructor.
    /// @param SLIPSerial Pointer to a SLIPEncodedUSBSerial object to use for communication to Eos.
    void initialize(SLIPEncodedUSBSerial* SLIPSerial, DataStorage* Storage){ slipSerial = SLIPSerial; storage = Storage; };

    /// @brief Performs all initialization of our communication with Eos.
    /// @attention Is blocking until a Serial connection is made.
    void begin()
    {
        // actually start serial
        slipSerial->begin(115200);
        // This is a hack around an Arduino bug. It was taken from the OSC library
	    //examples
	    while(!SerialUSB){}
        // this is necessary for reconnecting a device because it needs some time
        // for the serial port to open, but meanwhile the handshake message was
        // sent from Eos
        delay(5000);
        sendHandshakeReply();
    };

    /// @brief Checks for new received messages and handles them.
    void update()
    {
        // handle any receiving of packets and parsing of them
        readMessage();    

        // timeout logic
        // if we're connected but haven't heard from Eos in a bit, send a keepAlive ping.
        if(connected && !sentPing && (millis() - lastTimeReceived >= timeoutPingTime)){
            sendPing();
        }
        // if we still haven't heard from Eos, then after a while say that we've disconnected.
        if(millis() - lastTimeReceived >= timeoutDisconnectTime){
            connected = false;
        }

        // hail mary moment
        // if(!connected){
        //     sendHandshakeReply();
        //     connected = true;
        // }

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
        uint32_t index = wheel->getParameterIndex()+1; // Wheels are 1 indexed. thanks ETC!
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

    unsigned long getTimeSinceRX(){ return millis()-lastTimeReceived; };

    String getLastRXMessage(){return lastMsg;}

//////////////////////////////////////////// Receive Callbacks ////////////////////////////////////////////

    // handles "/eos/out/ping" messages
    void handlePingResponse(OSCMessage& msg, int matchedPatternOffset)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        // this lowkey doesn't have to do anything lol.
        // just have a function for completeness.2
        // noting that we're connected and updating our receive time is handled by prior functions.
        return;
    }
 
    // handles "/eos/out/active/chan" messages
    void handleChannelUpdate(OSCMessage& msg, int matchedPatternOffset)
    {
        char selectionBuffer[64];
        msg.getString(0, selectionBuffer, 32);
        String selectionString = String(selectionBuffer);

        uint valueIndexStart = selectionString.indexOf('[');
        uint valueIndexEnd = selectionString.indexOf(']');
        String selection = selectionString.substring(0,valueIndexStart-1); // -1 includes the seperating space
        String selectionValue = selectionString.substring(valueIndexStart+1,valueIndexEnd); // +1 as the start index is inclusive
        float value = selectionValue.toFloat();

        /*
            it is implied (and experimentally proven true)
            that the message to /eos/out/active/chan
            // comes before any messages to /eos/out/active/wheel/ *
            therefore, we clear the channel and param storage
            when we get a new channel selection
        */
        storage->clear();
        storage->setChannel(selection, value);
    }

    // // handles "/eos/out/param/*" messages
    // void handleParameterUpdate(OSCMessage& msg, int matchedPatternOffset)
    // {
    //     // have some fun to get the parameter name out of the address
    //     char paramNameBuffer[32];
    //     msg.getAddress(paramNameBuffer, matchedPatternOffset, 32);
    //     String paramName = String(paramNameBuffer);
    //     // pull level value from the payload
    //     float newValue = msg.getFloat(0);

        
    //     // find the correct parameter in our storage and store the new value
    //     int index = storage->find(paramName);
    //     if(index >= 0){
    //         storage->setParamValue(index, newValue);
    //     }
    // }

    // handles "/eos/out/active/wheel/*" messages
    void handleWheelUpdate(OSCMessage& msg, int matchedPatternOffset)
    {
        // pull the wheel index out of the address
        // we can get away with such a small buffer
        // bc the wheel index should be max 2 digits in basically all cases
        char wheelIndexBuffer[4];
        msg.getAddress(wheelIndexBuffer, matchedPatternOffset, 4);
        uint32_t index = String(wheelIndexBuffer).toInt();

        uint32_t category = msg.getInt(1);

        // get the parameter name and the value
        char paramNameBuffer[48];
        msg.getString(0, paramNameBuffer, 48);
        String paramNameString = String(paramNameBuffer);

        uint valueIndexStart = paramNameString.indexOf('[');
        uint valueIndexEnd = paramNameString.indexOf(']');
        String paramName = paramNameString.substring(0,valueIndexStart-1); // -1 includes the seperating space
        String paramValue = paramNameString.substring(valueIndexStart+1,valueIndexEnd); // +1 as the start index is inclusive
        float stringValue = paramValue.toFloat();

        /*
            there seems to be two ways to get the value of the parameter.
            one is passed in the string with the parameter name,
            which is the documented feature and operates similar to the /eos/out/active/channel
            However, there also seems to be an undocumented feature
            that provides the true value (similar to using the data key on the console) as a float argument in the message
            after the category argument.
        */
        float value = stringValue;
        // float value = msg.getFloat(2);

        // check if the wheel index has already been stored, as Eos sends packets every time the parameter updates
        if(storage->getParamCount() > (index-1)){
            // in this case we just care about updating the value
            storage->setParamValue((index-1), value);
            return; // early return so we don't create a new param
        }

        // create a new parameter object and populate it
        Parameter newParam;
        newParam.index = index;
        newParam.category = category; // this is a documented feature in Eos 2.6.0 and above
        newParam.name = paramName;
        newParam.value = value;

        storage->addParam(newParam);
    }
};