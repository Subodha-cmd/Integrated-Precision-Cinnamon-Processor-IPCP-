#include <Arduino.h>
#include <SoftwareSerial.h>
#include "JawController.h"

#define SW_RX  17
#define SW_TX  16
#define STEP_PIN 25
#define DIR_PIN  26
#define EN_PIN   27
#define R_SENSE  0.11f

SoftwareSerial SoftSerial(SW_RX, SW_TX);
JawController jaw(SoftSerial, R_SENSE, STEP_PIN, DIR_PIN, EN_PIN);

// Your real sensor logic goes here — e.g. reading a load cell.
bool myConditionCheck() {
  // return (scale1.get_units() > 50.0);
  return false;
}

void setup() {
  Serial.begin(9600);
  SoftSerial.begin(115200);

  jaw.begin(300);   // 300 = your calibrated STEPS_BETWEEN_POSITIONS
  jaw.setConditionCallback(myConditionCheck);

  Serial.println("Send 'h' to home, '1'=open, '2'=close, '3'=check_close");
}

void loop() {
  if (Serial.available() > 0) {
    char inChar = Serial.read();

    if (inChar == 'h') {
      bool ok = jaw.home();
      Serial.println(ok ? "Homed OK." : "HOMING FAILED — check sensor!");
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
    }
    else if (inChar == 'o') {
      jaw.jogOpen();
    }
    else if (inChar == 'c') {
      jaw.jogClose();
    }
  }
}