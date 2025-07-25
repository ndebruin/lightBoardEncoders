#pragma once

#include <Arduino.h>
#include <string.h>

#define encoderButtonDebounceTime 10
#define buttonDebounceTime 10

enum WheelMode
{
    Coarse,
    Fine
};


struct Parameter
{
    uint32_t index;
    String name;
    uint32_t category;
    float value;
};