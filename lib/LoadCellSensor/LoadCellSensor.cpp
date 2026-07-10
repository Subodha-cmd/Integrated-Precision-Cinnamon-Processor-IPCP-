#include "LoadCellSensor.h"

LoadCellSensor::LoadCellSensor(uint8_t doutPin, uint8_t sckPin)
  : _doutPin(doutPin), _sckPin(sckPin)
{
  // Actual hardware init happens in begin(), not here — same reasoning
  // as JawController: peripherals aren't safe to touch before setup().
}

void LoadCellSensor::begin() {
  _scale.begin(_doutPin, _sckPin);

  // Wait for the chip to be ready before taring — HX711 needs a brief
  // moment after power-up before its first reading is valid.
  while (!_scale.is_ready()) {
    delay(10);
  }

  _scale.set_scale();   // default scale of 1.0 until setCalibrationFactor() is called
  _scale.tare();        // zero out whatever the current (assumed empty) reading is
}

void LoadCellSensor::setCalibrationFactor(float factor) {
  _calibrationFactor = factor;
  _scale.set_scale(_calibrationFactor);
}

void LoadCellSensor::setThreshold(float thresholdGrams) {
  _thresholdGrams = thresholdGrams;
}

float LoadCellSensor::load_value() {
  if (!_scale.is_ready()) {
    return 0.000;   // avoid blocking/hanging if the chip isn't ready this instant
  }

  float grams = _scale.get_units(5);   // average of 5 readings, smooths out noise

  // Round to 3 decimal places
  float rounded = round(grams * 1000.0) / 1000.0;
  return rounded;
}

bool LoadCellSensor::load_on_cell() {
  return (load_value() > _thresholdGrams);
}

bool LoadCellSensor::load_on_cell_fast() {
  if (!_scale.is_ready()) return false;   // don't block if a conversion isn't ready yet
  float grams = _scale.get_units(1);      // single sample instead of 5 — ~100ms instead of ~500ms
  return (grams > _thresholdGrams);
}

void LoadCellSensor::tare() {
  _scale.tare();
}

long LoadCellSensor::getRawReading() {
  if (!_scale.is_ready()) {
    return 0;
  }
  return _scale.read_average(5);   // raw ADC value, unscaled — used only during calibration
}