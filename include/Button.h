#pragma once

#include <Arduino.h>

#include "Debouncer.h"

#define buttonDebounceTime 10

class Button {
  public:
    Button(uint8_t pin, unsigned long debounceMs = buttonDebounceTime, bool activeLow = true)
      : _pin(pin), _debounceMs(debounceMs), _activeLow(activeLow) {}

    void begin() {
      pinMode(_pin, _activeLow ? INPUT_PULLUP : INPUT);
      _lastReading = digitalRead(_pin);
      _stableState = _lastReading;
      _lastDebounceTime = millis();
    }

    // Call every loop(). Returns true exactly once per new press.
    bool pressed() {
      bool reading = digitalRead(_pin);
      bool triggered = false;

      if (reading != _lastReading) {
        _lastDebounceTime = millis();
      }

      if ((millis() - _lastDebounceTime) > _debounceMs) {
        if (reading != _stableState) {
          _stableState = reading;
          bool isPressed = _activeLow ? (_stableState == LOW) : (_stableState == HIGH);
          if (isPressed) {
            triggered = true;
          }
        }
      }

      _lastReading = reading;
      return triggered;
    }

  private:
    uint8_t _pin;
    unsigned long _debounceMs;
    bool _activeLow;
    bool _lastReading;
    bool _stableState;
    unsigned long _lastDebounceTime;
};