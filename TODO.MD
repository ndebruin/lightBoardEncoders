~~* Proper handling of empty string on `/eos/out/active/channel`~~
~~* Add iterator to `/eos/ping`~~
~~* add time-based updater for display~~
* Display Graphic Design
* re-write handleParameterUpdate()
* add pre-connection diagnostic/information display

* edge cases/unit testing?
  * there are large chunks of my own code that are very easily testable and just require creating/capturing proper test inputs

the following deal with the fact that plenty of RAM is available, but flash is at a premium
* remove vector.h requirement and just pre-allocate a sufficently large static array in DataStorage.h
	 (prob 64 or 128 large, but with oversize handling incase of some freaky fixture)
* replace Adafruit display driver with lighterweight U8g2 or U8x8 library


the following deal with porting to an STM32
* replace/supplement Encoder.h with one backed by the STM32's hardware timer encoder reading (look at STM32Duino HardwareTimer library)
* port OSC SLIPSerial to support STM32 native USB serial
* STM32 dev board environment profile
* try with an STM32 board? (can i find a nucleo or blue/blackpill?)

the following deal with moving to my custom board
* Test support with multiple encoders
* Implement parameter type selection w. buttons
* Implement multiple screens
