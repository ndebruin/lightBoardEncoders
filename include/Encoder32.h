/**
 * Encoder32.h - Rotary encoder library for STM32 (HAL) using hardware timer
 *             Encoder Mode. Header-only.
 *
 * Public API intentionally mirrors the Teensy Encoder library:
 *      int32_t read()
 *      void    write(int32_t p)
 *
 * Unlike the Teensy library, this does NOT bit-bang GPIO pins with
 * interrupts. It relies on the STM32 timer peripheral's hardware
 * quadrature decoder (Encoder Mode), so the counting itself is done
 * for free in silicon -- read()/write() just translate the raw timer
 * counter into a running 32-bit position, handling counter wraparound.
 *
 * See README.md for the exact CubeMX configuration steps.
 *
 * Usage:
 *
 *   #include "Arduino.h"
 *   #include "Encoder32.h"
 *
 *   extern TIM_HandleTypeDef htim3;   // set up by your timer init code
 *   void MX_TIM3_Init(void);
 *
 *   Encoder myEnc;                    // global -- no htim available yet
 *
 *   void setup() {
 *       MX_TIM3_Init();               // configure the timer + its pins
 *       myEnc.begin(&htim3);          // *then* hand it to Encoder
 *   }
 *
 *   void loop() {
 *       int32_t pos = myEnc.read();
 *       myEnc.write(0);
 *   }
 *
 * IMPORTANT: don't construct an Encoder with a TIM_HandleTypeDef at
 * global scope (e.g. `Encoder myEnc(&htim3);` as a global). Global
 * objects are constructed before setup() runs, i.e. before your timer
 * init code has configured htim3 -- at that point its registers are
 * still zeroed/garbage. Declare the Encoder with the default
 * constructor instead, and call begin() from inside setup(), after
 * the timer has been initialized.
 */

#pragma once
#define Encoder_h_

// #include <cstdint>

// This library does not assume any particular STM32 family.
//
// Target: Arduino framework (STM32duino / Arduino_Core_STM32). That
// core is itself built on top of the ST HAL, and "Arduino.h" pulls in
// the correct stm32<family>xx_hal.h for whichever board you've
// selected, so TIM_HandleTypeDef, HAL_TIM_Encoder_Start(), etc. are
// all available through this include.
//
// (If you're instead in a bare CubeMX/STM32CubeIDE project rather than
// Arduino, use "main.h" here instead -- CubeMX generates that file and
// it pulls in the same HAL headers via a different path.)
#include "Arduino.h"

class Encoder {
public:
    /**
     * Default constructor. Does no hardware access at all -- safe to
     * use for a global/static instance declared before setup() runs.
     * Call begin() once the timer has actually been initialized.
     */
    Encoder()
        : _htim(nullptr), _position(0), _lastCount(0), _range(0)
    {
    }

    /**
     * Optional convenience overload: equivalent to
     * `Encoder e; e.begin(htim);`. Only use this for an Encoder that
     * is constructed AFTER the timer has been initialized (e.g. a
     * local variable inside setup(), or a heap/stack object created
     * at runtime) -- not for a global, since globals are constructed
     * before setup() runs. If in doubt, use the default constructor
     * and call begin() explicitly instead.
     */
    explicit Encoder(TIM_HandleTypeDef* htim)
        : _htim(nullptr), _position(0), _lastCount(0), _range(0)
    {
        begin(htim);
    }

    /**
     * @param htim  Pointer to a TIM_HandleTypeDef that has already been
     *              configured (via CubeMX/MX_TIMx_Init, or hand-written
     *              init code) for Encoder Mode. Starts the timer's
     *              encoder interface and baselines the position.
     *
     * Call this from setup(), after your timer init function has run --
     * never before it.
     */
    void begin(TIM_HandleTypeDef* htim)
    {
        _htim = htim;

        // ARR defines where the hardware counter wraps (0 .. ARR, then
        // back to 0, or 0xFFFFFFFF -> 0 for a full 32-bit counter). We
        // need this to correctly interpret deltas across a wraparound.
        uint32_t arr = __HAL_TIM_GET_AUTORELOAD(_htim);
        _range = (uint64_t)arr + 1ULL; // e.g. 0x10000 for a 16-bit timer

        HAL_TIM_Encoder_Start(_htim, TIM_CHANNEL_ALL);

        _lastCount = __HAL_TIM_GET_COUNTER(_htim);
        _position = 0;
    }

    /**
     * @return current accumulated position, matching Teensy's
     *         Encoder::read(). Returns the last known position (0 if
     *         never begun) if begin() hasn't been called yet.
     */
    int32_t read()
    {
        if (_htim == nullptr) return _position;
        sync();
        return _position;
    }

    /**
     * Force the current position to an arbitrary value, matching
     * Teensy's Encoder::write(p). No-op on the hardware side if
     * begin() hasn't been called yet (just stores p).
     */
    void write(int32_t p)
    {
        _position = p;
        if (_htim != nullptr) {
            _lastCount = __HAL_TIM_GET_COUNTER(_htim);
        }
    }

    /**
     * Convenience extra (not required by the Teensy API, but handy):
     * reads the current position and resets it to zero atomically.
     */
    int32_t readAndReset()
    {
        if (_htim != nullptr) sync();
        int32_t p = _position;
        _position = 0;
        return p;
    }

private:
    TIM_HandleTypeDef* _htim;
    volatile int32_t   _position;   // last computed absolute position
    volatile uint32_t  _lastCount;  // raw CNT value at last read/write
    uint64_t           _range;      // (ARR + 1) as uint64_t, for modulus math

    // Pulls the current hardware counter, folds the delta since the
    // last call into _position (correctly handling wraparound even for
    // 16-bit timers), and updates _lastCount.
    void sync()
    {
        uint32_t count = __HAL_TIM_GET_COUNTER(_htim);

        // Raw difference since last sync, before unwrapping.
        int64_t delta = (int64_t)count - (int64_t)_lastCount;

        // Fold into the shortest signed path around the counter's
        // range, so a wrap from e.g. 0xFFFF -> 0x0000 is seen as +1,
        // not -65535.
        if (delta > (int64_t)(_range / 2)) {
            delta -= (int64_t)_range;
        } else if (delta < -(int64_t)(_range / 2)) {
            delta += (int64_t)_range;
        }

        _position += (int32_t)delta;
        _lastCount = count;
    }
};

// #endif // ENCODER_H