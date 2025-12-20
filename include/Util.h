#pragma once

#include <Arduino.h>
#include <string.h>

#define encoderButtonDebounceTime 10
#define buttonDebounceTime 10

#define timeoutPingTime 10000 // 10 second
#define timeoutDisconnectTime 30000 // 30 seconds

enum WheelMode
{
    Coarse,
    Fine
};


struct Parameter
{
    int32_t index;
    String name;
    int32_t category;
    float value;
};