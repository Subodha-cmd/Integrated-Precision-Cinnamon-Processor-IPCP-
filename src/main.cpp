#include <Arduino.h>
#include "LoadCellSensor.h"

// --- Pin assignments from your schematic ---
#define HX711_1_DOUT  18
#define HX711_1_SCK   19
#define HX711_2_DOUT  32
#define HX711_2_SCK   33

LoadCellSensor loadCell1(HX711_1_DOUT, HX711_1_SCK);
LoadCellSensor loadCell2(HX711_2_DOUT, HX711_2_SCK);

// Store the "empty" (tared) raw baseline for each cell, taken once at boot.
long rawEmpty1 = 0;
long rawEmpty2 = 0;

void setup() {
  Serial.begin(9600);
  delay(1000);

  Serial.println("=== HX711 Calibration Mode ===");
  Serial.println("Make sure BOTH load cells are completely EMPTY right now.");
  Serial.println("Taring in 3 seconds...");
  delay(3000);

  loadCell1.begin();   // begin() already tares internally
  loadCell2.begin();

  // Record the raw baseline right after taring, for the calibration formula.
  rawEmpty1 = loadCell1.getRawReading();
  rawEmpty2 = loadCell2.getRawReading();

  Serial.println("Tare complete.");
  Serial.print("Load Cell 1 empty baseline: ");
  Serial.println(rawEmpty1);
  Serial.print("Load Cell 2 empty baseline: ");
  Serial.println(rawEmpty2);

  Serial.println();
  Serial.println("--- Instructions ---");
  Serial.println("1. Place a KNOWN WEIGHT on Load Cell 1.");
  Serial.println("2. Send '1' to read its current raw value.");
  Serial.println("3. Repeat steps 1-2 for Load Cell 2 using '2'.");
  Serial.println("4. Send 't' anytime to re-tare both (remove all weight first!).");
  Serial.println("--------------------");
}

void loop() {
  if (Serial.available() > 0) {
    char inChar = Serial.read();

    if (inChar == '1') {
      long raw = loadCell1.getRawReading();
      Serial.print("Load Cell 1 raw (with weight): ");
      Serial.print(raw);
      Serial.print("   | Difference from empty: ");
      Serial.println(raw - rawEmpty1);
      Serial.println("Calibration factor = (this difference) / (known weight in grams)");
    }
    else if (inChar == '2') {
      long raw = loadCell2.getRawReading();
      Serial.print("Load Cell 2 raw (with weight): ");
      Serial.print(raw);
      Serial.print("   | Difference from empty: ");
      Serial.println(raw - rawEmpty2);
      Serial.println("Calibration factor = (this difference) / (known weight in grams)");
    }
    else if (inChar == 't') {
      Serial.println("Re-taring both — make sure they are EMPTY...");
      loadCell1.tare();
      loadCell2.tare();
      rawEmpty1 = loadCell1.getRawReading();
      rawEmpty2 = loadCell2.getRawReading();
      Serial.println("Re-tare complete.");
    }
  }
}