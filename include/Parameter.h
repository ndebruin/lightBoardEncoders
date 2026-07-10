#pragma once

#include <Arduino.h>

enum Category
{
    None = 0,
    Intensity = 1,
    Focus = 2,
    Color = 3,
    Shutter =6,
    Image = 4,
    Form = 5,
};

struct Parameter
{
    int32_t index;
    String name;
    Category category;
    float value;
};
