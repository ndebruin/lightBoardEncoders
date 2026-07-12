~~* Proper handling of empty string on `/eos/out/active/channel`~~
~~* Add iterator to `/eos/ping`~~
~~* add time-based updater for display~~
~~* port OSC SLIPSerial to support STM32 native USB serial~~
~~* re-write handleParameterUpdate()~~
~~* remove vector.h requirement and just pre-allocate a sufficently large static array in DataStorage.h~~
~~* Test support with multiple encoders~~
~~* Implement parameter type selection w. buttons~~

* Display Graphic Design
* add pre-connection diagnostic/information display


the following deal with porting to an STM32
* replace/supplement Encoder.h with one backed by the STM32's hardware timer encoder reading (look at STM32Duino HardwareTimer library)
* STM32 dev board environment profile
* try with an STM32 board? (can i find a nucleo or blue/blackpill?)

the following deal with moving to my custom board

* Implement multiple screens
