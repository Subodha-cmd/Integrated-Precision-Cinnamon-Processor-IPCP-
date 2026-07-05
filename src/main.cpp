#include <Arduino.h>
#include "LoadCellSensor.h"
#include "JawController.h"
#include <SoftwareSerial.h>

#define HX711_1_DOUT  18
#define HX711_1_SCK   19
#define HX711_2_DOUT  32
#define HX711_2_SCK   33

#define SW_RX  17
#define SW_TX  16
#define STEP_PIN 25
#define DIR_PIN  26
#define EN_PIN   27
#define R_SENSE  0.11f

LoadCellSensor loadCell1(HX711_1_DOUT, HX711_1_SCK);   // grip-detection sensor (drives homing/closing)
LoadCellSensor loadCell2(HX711_2_DOUT, HX711_2_SCK);   // weight-measurement only

SoftwareSerial SoftSerial(SW_RX, SW_TX);
JawController jaw(SoftSerial, R_SENSE, STEP_PIN, DIR_PIN, EN_PIN);

// Only loadCell1 decides when the jaw should stop (homing + check_close).
// loadCell2 is read separately, purely for reporting the actual grading weight.
bool myConditionCheck() {
  return loadCell1.load_on_cell();
}

void setup() {
  Serial.begin(9600);
  SoftSerial.begin(115200);

  Serial.println("Taring both load cells — make sure they are EMPTY...");
  loadCell1.begin();
  loadCell2.begin();

  loadCell1.setCalibrationFactor(210);   // your real calibrated value
  loadCell2.setCalibrationFactor(210);   // your real calibrated value (likely different per cell)

  loadCell1.setThreshold(5.0);           // grip-detection threshold, calibrated for your mechanism

  jaw.begin(300);   // your calibrated STEPS_BETWEEN_POSITIONS
  jaw.setConditionCallback(myConditionCheck);

  Serial.println("Send 'h' to home, '1'=open, '2'=close, '3'=check_close, 'w' for weight");
}

void loop() {
  if (Serial.available() > 0) {
    char inChar = Serial.read();

    if (inChar == 'h') {
      bool ok = jaw.home();
      Serial.println(ok ? "Homed OK." : "HOMING FAILED — check load cell 1!");
    }
    else if (inChar == '1') {
      jaw.openJaw();
      Serial.println("Jaw OPEN.");
    }
    else if (inChar == '2') {
      jaw.closeJaw();
      Serial.println("Jaw CLOSED.");
    }
    else if (inChar == '3') {
      long result = jaw.checkClose();
      Serial.print("check_close() returned: ");
      Serial.println(result);

      // Once the jaw has stopped (gripped something, presumably), read
      // the actual weight from loadCell2 — the dedicated measurement sensor.
      Serial.print("Measured weight (load cell 2): ");
      Serial.print(loadCell2.load_value(), 3);
      Serial.println(" g");
    }
    else if (inChar == 'w') {
      Serial.print("LC1 (grip sensor) grams: ");
      Serial.print(loadCell1.load_value(), 3);
      Serial.print("   LC2 (weight sensor) grams: ");
      Serial.println(loadCell2.load_value(), 3);
    }
  }
}