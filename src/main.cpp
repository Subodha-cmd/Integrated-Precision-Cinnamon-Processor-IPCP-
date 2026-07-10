#include <Arduino.h>
#include "LoadCellSensor.h"

// --- Pin assignments (per your final schematic) ---
#define HX711_1_DOUT  18
#define HX711_1_SCK   19
#define HX711_2_DOUT  32
#define HX711_2_SCK   33

LoadCellSensor loadCell1(HX711_1_DOUT, HX711_1_SCK);
LoadCellSensor loadCell2(HX711_2_DOUT, HX711_2_SCK);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("=== Load Cell Live Readings ===");
  Serial.println("Taring both — make sure they are EMPTY...");
  delay(2000);

  loadCell1.begin();   // begin() tares internally
  loadCell2.begin();

  // --- Calibration factors you found ---
  loadCell1.setCalibrationFactor(1005);
  loadCell2.setCalibrationFactor(-1100);

  // --- Threshold for load_on_cell() ---
  loadCell1.setThreshold(10.0);
  loadCell2.setThreshold(10.0);

  Serial.println("Tare complete. Starting live readings...");
  Serial.println("--------------------------------------------");
}

void loop() {
  float weight1 = loadCell1.load_value();
  float weight2 = loadCell2.load_value();

  bool onCell1 = loadCell1.load_on_cell();
  bool onCell2 = loadCell2.load_on_cell();

  Serial.print("LC1: ");
  Serial.print(weight1, 3);
  Serial.print(" g  | load_on_cell(): ");
  Serial.print(onCell1 ? "TRUE" : "FALSE");

  Serial.print("     LC2: ");
  Serial.print(weight2, 3);
  Serial.print(" g  | load_on_cell(): ");
  Serial.println(onCell2 ? "TRUE" : "FALSE");

  delay(500);
}