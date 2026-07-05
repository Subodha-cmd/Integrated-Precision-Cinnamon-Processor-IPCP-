#pragma once
#include <Arduino.h>
#include <HX711.h>

class LoadCellSensor {
  public:
    // doutPin/sckPin: the HX711 module's DT and SCK pins
    LoadCellSensor(uint8_t doutPin, uint8_t sckPin);

    // Call once in setup(). Starts the HX711 and tares (zeros) it —
    // IMPORTANT: nothing should be resting on the load cell when this runs.
    void begin();

    // Sets the calibration factor found during calibration (see calibrateStep()).
    // Once set, load_value() returns real grams.
    void setCalibrationFactor(float factor);

    // Sets the threshold (in grams) used by load_on_cell() to decide true/false.
    void setThreshold(float thresholdGrams);

    // Returns the current reading in grams, rounded to 3 decimal places.
    float load_value();

    // Returns true if the current load exceeds the calibrated threshold.
    bool load_on_cell();

    // Re-zeros the scale at its current (assumed empty) state.
    void tare();

    // --- Calibration helper ---
    // Prints the raw reading to Serial, for use during the one-time
    // calibration process (see explanation in chat / README).
    long getRawReading();

  private:
    HX711  _scale;
    uint8_t _doutPin;
    uint8_t _sckPin;
    float   _calibrationFactor = 1.0;   // placeholder until calibrated
    float   _thresholdGrams    = 5.0;   // placeholder default, calibrate for real use
};