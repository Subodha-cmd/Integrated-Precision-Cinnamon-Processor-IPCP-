#include "JawController.h"

JawController::JawController(Stream &uartPort, float rSense,
                              uint8_t stepPin, uint8_t dirPin, uint8_t enPin)
  : _driver(&uartPort, rSense),      // construct the TMC2208 driver bound to the given serial stream
    _stepper(1, stepPin, dirPin),    // construct AccelStepper in "driver mode" (STEP+DIR only)
    _enPin(enPin)
{
  // Nothing else needed here — real hardware setup happens in begin(),
  // since Arduino peripherals aren't safe to touch before setup() runs.
}

void JawController::begin(long stepsBetweenPositions,
                           int rmsCurrent,
                           float holdMultiplier,
                           int microsteps) {
  _stepsBetweenPositions = stepsBetweenPositions;

  // --- Enable the driver (active-LOW EN pin) ---
  pinMode(_enPin, OUTPUT);
  digitalWrite(_enPin, LOW);

  // --- Configure the TMC2208 over UART ---
  _driver.begin();
  _driver.toff(5);                              // required non-zero value to enable output stage
  _driver.rms_current(rmsCurrent, holdMultiplier); // running current + hold-current fraction (heat control)
  _driver.iholddelay(10);                        // smooth ramp-down to hold current after last step
  _driver.microsteps(microsteps);
  _driver.en_spreadCycle(false);                 // start in StealthChop (quiet) by default
  _driver.pwm_autoscale(true);                   // required for StealthChop to behave correctly

  // --- Motion profile ---
  _stepper.setMaxSpeed(_normalMaxSpeed);
  _stepper.setAcceleration(1000.0);
}

void JawController::setConditionCallback(ConditionCallback callback) {
  _conditionCallback = callback;
}

void JawController::stepOnce(int direction) {
  // Queue exactly one more step in the given direction, then run until
  // that single step is actually executed (AccelStepper is non-blocking
  // per call, so we loop run() until this one step is done).
  _stepper.move(direction);
  while (_stepper.distanceToGo() != 0) {
    _stepper.run();
  }
}

bool JawController::home(int homingSpeed, int maxStepsMultiplier) {
  if (_conditionCallback == nullptr) {
    // No sensor logic attached — homing would run forever with nothing
    // to stop it, so refuse outright rather than risk grinding the mechanism.
    return false;
  }

  long maxHomingSteps = _stepsBetweenPositions * maxStepsMultiplier;

  _stepper.setMaxSpeed(homingSpeed);   // slow, controlled speed specifically for homing

  long stepsTaken = 0;
  bool triggered = false;

  while (!triggered && stepsTaken < maxHomingSteps) {
    stepOnce(1);   // one step in the CLOSE direction
    stepsTaken++;

    if (_conditionCallback()) {
      triggered = true;
    }
  }

  _stepper.setMaxSpeed(_normalMaxSpeed);   // restore normal speed regardless of outcome

  if (!triggered) {
    _homed = false;
    return false;   // safety abort — condition never fired within the allowed range
  }

  // This physical spot is now our verified closed reference.
  // We pick 0 as an arbitrary internal reference for "closed" — the exact
  // number doesn't matter, only its distance to "open" does.
  _closedPosition = 0;
  _stepper.setCurrentPosition(_closedPosition);
  _openPosition = _closedPosition - _stepsBetweenPositions;

  _homed = true;
  return true;
}

void JawController::openJaw() {
  _stepper.moveTo(_openPosition);
  while (_stepper.distanceToGo() != 0) {
    _stepper.run();
  }
}

void JawController::closeJaw() {
  _stepper.moveTo(_closedPosition);
  while (_stepper.distanceToGo() != 0) {
    _stepper.run();
  }
}

long JawController::checkClose() {
  if (_conditionCallback == nullptr) {
    return -1;   // no sensor attached — caller should check for this error case
  }

  openJaw();   // always start from the known, fully-open reference point

  int direction = (_closedPosition > _openPosition) ? 1 : -1;

  while (_stepper.currentPosition() != _closedPosition) {
    stepOnce(direction);

    if (_conditionCallback()) {
      return abs(_closedPosition - _stepper.currentPosition());
    }
  }

  return 0;   // reached full closure without the condition ever triggering
}

void JawController::jogOpen(int steps) {
  _stepper.move(-steps);
  while (_stepper.distanceToGo() != 0) {
    _stepper.run();
  }
}

void JawController::jogClose(int steps) {
  _stepper.move(steps);
  while (_stepper.distanceToGo() != 0) {
    _stepper.run();
  }
}

long JawController::getCurrentPosition() {
  return _stepper.currentPosition();
}

long JawController::getOpenPosition() {
  return _openPosition;
}

long JawController::getClosedPosition() {
  return _closedPosition;
}

bool JawController::isHomed() {
  return _homed;
}