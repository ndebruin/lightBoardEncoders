#include "EosComms.h"

namespace EosComms
{

//////////////////////////////////////////// PRIVATE ////////////////////////////////////////////
// private has to come first bc this is a source file and the C++ preprocessor is stupid
    namespace { // this creates a functional equivalent of "private:" in a class
        SLIPEncodedUSBSerial* _slipSerial; // serial object
        DataStorage* _storage;
        
        bool _connected = false;
        bool _sentPing = false;
        unsigned long _pingIterator = 0;

        unsigned long _lastTimeReceived;
        unsigned long _lastTimeSent;

        String _curMsg;
        String _lastMsg;

        /// @brief Sends an OSCMessage object over our SLIPSerial object.
        void sendMessage(OSCMessage& msg)
        {
            // char buff[80];
            // msg.getAddress(buff, 0, 80);
            // Serial1.println(buff);
            _slipSerial->beginPacket();
            msg.send(*_slipSerial);
            _slipSerial->endPacket();
            _lastTimeSent = millis(); // update our timers
        }

        /// @brief  Sends the handshake reply. Notably not an OSC message.
        void sendHandshakeReply()
        {
            _slipSerial->beginPacket();
            _slipSerial->write((const uint8_t*)HANDSHAKE_REPLY.c_str(), HANDSHAKE_REPLY.length());
            _slipSerial->endPacket();
            _lastTimeSent = millis();
        }

        /// @brief Send message to "/eos/ping". No payload.
        void sendPing()
        {
            _sentPing = true;
            OSCMessage ping("/eos/ping");
            ping.add((int32_t)_pingIterator);
            sendMessage(ping);
            _pingIterator++;
        }

        /// @brief Sends all required filters to Eos in a single OSC Message.
        void IssueFilters()
        {
            // this tells Eos to only send us specific messages
            OSCMessage filter("/eos/filter/add");
            filter.add("/eos/out/active/chan"); // selected channel details
            filter.add("/eos/out/active/wheel/*"); // control parameters of the channel
            filter.add("/eos/out/ping"); // ping back messages
            sendMessage(filter);
        }

        /// @brief Handles routing OSC messages to the correct callback functions.
        /// @param msg The OSCMessage object to route.
        void routeOSCMessage(OSCMessage& msg)
        {
            msg.route("/eos/out/ping",handlePingResponse); // ping responses
            msg.route("/eos/out/active/chan",handleChannelUpdate); // selected channel details
            msg.route("/eos/out/active/wheel",handleWheelUpdate); // control parameters of the channel
        }

        /// @brief Handles a received generic string.
        /// @param msg String object that contains a received message.
        void parseMessage(String& msg)
        {
            // check to see if this is the handshake string
            if (msg.indexOf(HANDSHAKE_QUERY) != -1){
                // handshake string found!
                sendHandshakeReply();
                _connected = true;
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
            size = _slipSerial->available();
            if(size > 0){
                // fill the message with all available bytes
                while(size--){
                    _curMsg += (char)(_slipSerial->read());
                }
            }
            // if we've gotten a whole packet of data, go handle it
            if(_slipSerial->endofPacket()){
                // update our lastReceived timers
                _lastTimeReceived = millis();
                _connected = true;
                _sentPing = false;
                _lastMsg = _curMsg;

                // parse our message
                parseMessage(_curMsg);
                _curMsg = String();
            }
            

            
        }
        
    }; // end of empty namespace (equivalent to "private:" for a class)
//////////////////////////////////////////// END OF PRIVATE /////////////////////////////////////////////





//////////////////////////////////////////// Basic Functions ////////////////////////////////////////////

    /// @brief This replaces an equivalent class constructor.
    /// @param SLIPSerial Pointer to a SLIPEncodedUSBSerial object to use for communication to Eos.
    void initialize(SLIPEncodedUSBSerial* SLIPSerial, DataStorage* storage){ _slipSerial = SLIPSerial; _storage = storage; };

    /// @brief Performs all initialization of our communication with Eos.
    /// @attention Is blocking until a Serial connection is made.
    void begin()
    {
        // actually start serial
        _slipSerial->begin(115200);
        // This is a hack around an Arduino bug. It was taken from the OSC library
	    //examples
	    while(!SerialUSB){}
        // this is necessary for reconnecting a device because it needs some time
        // for the serial port to open, but meanwhile the handshake message was
        // sent from Eos
        delay(2500);
        sendHandshakeReply();
    };

    /// @brief Checks for new received messages and handles them.
    void update()
    {
        // handle any receiving of packets and parsing of them
        readMessage();    

        // timeout logic
        // if we're connected but haven't heard from Eos in a bit, send a keepAlive ping.
        if(_connected && !_sentPing && (millis() - _lastTimeReceived >= timeoutPingTime)){
            sendPing();
        }
        // if we still haven't heard from Eos, then after a while say that we've disconnected.
        if(millis() - _lastTimeReceived >= timeoutDisconnectTime){
            _connected = false;
        }

    };

//////////////////////////////////////////// Transmission Functions ////////////////////////////////////////////

    /// @brief Issues a subscribe to Eos for a specific Parameter object
    /// @param param Parameter object (name) to subscribe to updates for.
    void Subscribe(Parameter param)
    {
        OSCMessage sub(String("/eos/subscribe/param/"+param.name).c_str());
        sub.add(1); // magic number to say subscribe to parameter at this path. Top of P6 of "Supported OSC Commands.pdf"
        sendMessage(sub);
    }

    /// @brief Issues an unsubscribe to Eos for a specific Parameter object
    /// @param param Parameter object (name) to unsubscribe from updates for.
    void Unsubscribe(Parameter param)
    {
        OSCMessage unsub(String("/eos/subscribe/param/"+param.name).c_str());
        unsub.add(0); // magic number to say un-subscribe to parameter at this path. Top of P6 of "Supported OSC Commands.pdf"
        sendMessage(unsub);
    }

    /// @brief Sends command data for a given wheel object when called.
    /// @param wheel The Wheel object you wish to send command data for.
    void sendWheelData(Wheel* wheel)
    {
        if(wheel->getParameterIndex() >= _storage->getParamCount()){return;};
        
        uint32_t index = _storage->getParam(wheel->getParameterIndex()).index;        
        float val = wheel->getCommand();

        // create the OSC address
        String addr = "/eos/active/wheel/";
        // route to either coarse or fine paths
        if(wheel->getMode() == WheelMode::Coarse){ addr.concat("coarse/"); }
        else if(wheel->getMode() == WheelMode::Fine){ addr.concat("fine/"); }

        // add the eos wheel index
        addr.concat(index);

        // create the actual OSCMessage object and add our data
        OSCMessage msg(addr.c_str());
        msg.add((float)val); // use a float cast on a float variable just to be safe and make sure the overridden add() function uses the correct datatype

        sendMessage(msg); // send it!
    }

//////////////////////////////////////////// Utility Functions ////////////////////////////////////////////

    bool isConnected(){return _connected;};

    unsigned long getTimeSinceRX(){ return millis()-_lastTimeReceived; };

    String getLastRXMessage(){return _lastMsg;}

//////////////////////////////////////////// Receive Callbacks ////////////////////////////////////////////

    // handles "/eos/out/ping" messages
    void handlePingResponse(OSCMessage& msg, int matchedPatternOffset)
    {
        // this lowkey doesn't have to do anything lol.
        // just have a function for completeness/consistency.
        // noting that we're connected and updating our receive time is handled by prior functions.
        return;
    }
 
    // handles "/eos/out/active/chan" messages
    void handleChannelUpdate(OSCMessage& msg, int matchedPatternOffset)
    {
        char selectionBuffer[80];
        msg.getString(0, selectionBuffer, 80);
        String selectionString = String(selectionBuffer);

        // this handles the case where an empty string is sent with the message, which signifies the de-selection of all channels
        // example: when you clear the command line it sends this (which is a little goofy but ok) 
        if(selectionString.equals("")){ 
            _storage->clearChannel();
            return;
        }

        uint valueIndexStart = selectionString.indexOf('[');
        uint valueIndexEnd = selectionString.indexOf(']');
        String selection = selectionString.substring(0,valueIndexStart-2); // -1 includes the seperating space
        String selectionValue = selectionString.substring(valueIndexStart+1,valueIndexEnd); // +1 as the start index is inclusive
        float value = selectionValue.toFloat();

        _storage->setChannel(selection, value);
    }

    // handles "/eos/out/active/wheel/*" messages
    void handleWheelUpdate(OSCMessage& msg, int matchedPatternOffset)
    {
        // pull the wheel index out of the address
        // we can get away with such a small buffer
        // bc the wheel index should be max 2 digits in basically all cases
        char wheelIndexBuffer[4];
        msg.getAddress(wheelIndexBuffer, matchedPatternOffset+1, 4);
        uint32_t index = String(wheelIndexBuffer).toInt();
             
        int32_t category = msg.getInt(1);

        int16_t paramIndex = _storage->find(index);

        // if it's a param being removed, then the category is 0
        // determined this behavior to be true experimentally, is not documented
        // this is a heuristic
        if(category == 0){
            _storage->removeParam(index);
            return;
        }

        // get the parameter name and the value
        char paramNameBuffer[48];
        msg.getString(0, paramNameBuffer, 48);
        String paramNameString = String(paramNameBuffer);

        uint valueIndexStart = paramNameString.indexOf('[');
        uint valueIndexEnd = paramNameString.indexOf(']');
        String paramName = paramNameString.substring(0,valueIndexStart-1); // -1 includes the seperating space
        String paramValue = paramNameString.substring(valueIndexStart+1,valueIndexEnd); // +1 as the start index is inclusive

        /*
            there seems to be two ways to get the value of the parameter.
            one is passed in the string with the parameter name,
            which is the documented feature and operates similar to the /eos/out/active/channel
            However, there also seems to be an undocumented feature
            that provides the true value (similar to using the data key on the console) as a float argument in the message
            after the category argument.
        */
        // float value = paramValue.toFloat();
        float value = msg.getFloat(2);

        

        // if we don't already have a param at this index
        if(paramIndex == -1){
            // add a new parameter to our storage
            _storage->addParam(index, paramName, category, value);
        }
        // if there is a param at this index but it's not the same as our new one
        else if(!_storage->getParam(paramIndex).name.equals(paramName)){
            _storage->removeParam(index); // remove the previous one
            _storage->addParam(index, paramName, category, value); // add the new one
        }
        else{
            _storage->setParamValue(paramIndex, value); // in this case we just update the value of the param
        }

        return;
    }
};