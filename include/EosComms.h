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

/*
    due to weirdness with namespaces we only declare the functions
    that would be equivalent to public methods in a class
    in this header file.
    all the private stuff (being stored in an unnamed namespace)
    is declared and defined in the source file.

    why are we going through all this effort to use namespaces,
    rather than just using a class????

    C/C++/Arduino doesn't have the concept of a static class.
    
    but the concept of a "static class" makes the most sense 
    for the functionality of this code,
    as you're only ever going to have one connection to Eos.
    
    having a "static class" is also crucial for some of the functionality
    of the OSC library, as it doesn't support the C++ std::function and std::bind,
    meaning that the callbacks for dispatch() and route() 
    can't be instance methods (methods of an object)
    as there's no way to pass the context (functionally the "this.") to them.
    if the OSC library supported the C++ std::function and std::bind,
    then we could use those to encapsulate the context
    and wouldn't have to do all this mess.
    
    but it doesn't, and i don't want to modify it, so here we are....

    i guess i could also just reimplement the dispatch() and route() functions,
    but nah.
*/

namespace EosComms
{
//////////////////////////////////////////// Basic Functions ////////////////////////////////////////////

    /// @brief This replaces an equivalent class constructor.
    /// @param SLIPSerial Pointer to a SLIPEncodedUSBSerial object to use for communication to Eos.
    void initialize(SLIPEncodedUSBSerial* SLIPSerial);

    /// @brief Performs all initialization of our communication with Eos.
    /// @attention Is blocking until a Serial connection is made.
    void begin();

    /// @brief Checks for new received messages and handles them.
    void update();

//////////////////////////////////////////// Transmission Functions ////////////////////////////////////////////

    /// @brief Issues a subscribe to Eos for a specific Parameter object
    /// @param param Parameter object (name) to subscribe to updates for.
    void Subscribe(Parameter param);

    /// @brief Issues an unsubscribe to Eos for a specific Parameter object
    /// @param param Parameter object (name) to unsubscribe from updates for.
    void Unsubscribe(Parameter param);

    /// @brief Sends command data for a given wheel object when called.
    /// @param wheel The Wheel object you wish to send command data for.
    void sendWheelData(Wheel* wheel);

//////////////////////////////////////////// Utility Functions ////////////////////////////////////////////

    bool isConnected();

//////////////////////////////////////////// Receive Callbacks ////////////////////////////////////////////

    /// @brief Logic for when we receive a ping back from Eos.
    /// @param msg OSCMessage object which is addressed for our callback.
    void handlePingResponse(OSCMessage& msg);

    /// @brief Logic for updating selection data when we get an update on channel selection from Eos.
    /// @param msg OSCMessage object which is addressed for our callback.
    void handleChannelUpdate(OSCMessage& msg);

    /// @brief Logic for updating parameter values when we get an update for a parameter from Eos.
    /// @param msg OSCMessage object which is addressed for our callback.
    void handleParameterUpdate(OSCMessage& msg, int patternOffset);

    /// @brief Logic for updating parameter wheels when we get an update on channel selection from Eos.
    /// @param msg OSCMessage object which is addressed for our callback.
    void handleWheelUpdate(OSCMessage& msg, int patternOffset);
};