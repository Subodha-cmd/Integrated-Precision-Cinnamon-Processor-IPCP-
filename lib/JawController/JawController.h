#pragma once
#include <Arduino.h>
#include <TMCStepper.h>
#include <AccelStepper.h>

// A function pointer type for the sensor condition check.
// Your main.cpp will provide a real function matching this signature
// (e.g. one that reads a load cell and returns true past a threshold).
typedef bool (*ConditionCallback)();

class JawController {
  public:
    // uartPort: pass your SoftwareSerial (or HardwareSerial) object, e.g. SoftSerial
    // rSense: the driver board's current-sense resistor value (ohms)
    // stepPin/dirPin/enPin: your STEP/DIR/EN GPIOs
    JawController(Stream &uartPort, float rSense,
                  uint8_t stepPin, uint8_t dirPin, uint8_t enPin);

    // Call once in setup(). Configures the TMC2208 driver and motion profile.
    // stepsBetweenPositions: calibrated distance between open and closed (always positive)
    void begin(long stepsBetweenPositions,
               int rmsCurrent = 500,
               float holdMultiplier = 0.4,
               int microsteps = 16);

    // Give the library your real sensor-check function (load cell threshold, etc.)
    // This same function is used both for homing (detecting the hard stop)
    // and for checkClose() (detecting an early grip) — pass whichever logic applies.
    void setConditionCallback(ConditionCallback callback);

    // Drives in the CLOSE direction, one step at a time, until the condition
    // callback triggers — that point becomes the verified CLOSED_POSITION,
    // and OPEN_POSITION is derived from it using stepsBetweenPositions.
    // Returns true if homing succeeded, false if it hit the safety step limit
    // without the condition ever triggering (sensor fault / wiring issue).
    bool home(int homingSpeed = 200, int maxStepsMultiplier = 2);

    // Moves to the open position and blocks until arrived.
    void openJaw();

    // Moves to the closed position and blocks until arrived.
    void closeJaw();

    // Opens fully, then steps toward closed one microstep at a time,
    // checking the condition callback after every step.
    // Returns steps remaining if triggered early, or 0 if fully closed.
    long checkClose();

    // Manual jog helpers, useful for calibration via Serial commands.
    void jogOpen(int steps = 50);
    void jogClose(int steps = 50);

    // Read-only access to current state, useful for logging/diagnostics.
    long getCurrentPosition();
    long getOpenPosition();
    long getClosedPosition();
    bool isHomed();

  private:
    TMC2208Stepper _driver;
    AccelStepper   _stepper;
    uint8_t        _enPin;

    long _stepsBetweenPositions = 0;
    long _openPosition   = 0;
    long _closedPosition = 0;
    bool _homed           = false;

    float _normalMaxSpeed = 2000.0;

    ConditionCallback _conditionCallback = nullptr;

    // Internal helper: moves exactly one step in the given direction (+1 or -1)
    // and blocks until that single step completes.
    void stepOnce(int direction);
};