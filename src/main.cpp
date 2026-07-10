#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "JawController.h"
#include "LoadCellSensor.h"

// --- Motor driver pins ---
#define SW_RX     17
#define SW_TX     16
#define STEP_PIN  25
#define DIR_PIN   26
#define EN_PIN    27
#define R_SENSE   0.11f

// --- Load cell pins ---
#define HX711_1_DOUT  18   // Load Cell 1: cinnamon presence + weight
#define HX711_1_SCK   19
#define HX711_2_DOUT  32   // Load Cell 2: jaw grip-contact sensor
#define HX711_2_SCK   33

// --- OLED pins/config ---
#define SDA_PIN       21
#define SCL_PIN       22
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS  0x3C

// ============================================================
// CALIBRATION CONSTANTS — adjust these to your real hardware
// ============================================================
#define STEPS_BETWEEN_POSITIONS   300     // calibrated open<->closed travel
#define RUN_CURRENT_MA            500
#define LOADCELL1_CAL_FACTOR      1005
#define LOADCELL2_CAL_FACTOR      -1100
#define LOADCELL1_THRESHOLD_G     1    // presence/weight threshold
#define LOADCELL2_THRESHOLD_G     20.0    // grip-contact threshold

#define STEPS_PER_MM  10.54   // <-- X from your notes. HARD-CODED, must be calibrated for real mechanism.

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool oledReady = false;

SoftwareSerial SoftSerial(SW_RX, SW_TX);
JawController jaw(SoftSerial, R_SENSE, STEP_PIN, DIR_PIN, EN_PIN);

LoadCellSensor loadCell1(HX711_1_DOUT, HX711_1_SCK);
LoadCellSensor loadCell2(HX711_2_DOUT, HX711_2_SCK);

// The jaw's own condition (used by home() and checkClose()) is the
// GRIP sensor (load cell 2) — this is what tells the jaw "you just
// touched something, stop closing."
bool jawTouchCondition() {
  return loadCell2.load_on_cell_fast();   // fast path during motion
}

// ------------------------------------------------------------
// OLED helpers
// ------------------------------------------------------------
void oledMessage(const String &line1, const String &line2 = "", const String &line3 = "", int textSize = 2) {
  if (!oledReady) return;
  display.clearDisplay();
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(line1);
  if (line2.length() > 0) {
    display.println(line2);
  }
  if (line3.length() > 0) {
    display.println(line3);
  }
  display.display();
}
// ------------------------------------------------------------
// gradeFromDiameter()
// Implements the grading table from your notes exactly.
// D is in millimeters. Returns the grade label as a String.
// ------------------------------------------------------------
String gradeFromDiameter(float D) {
  if (D <= 0)          return "No Object";     // jaw closed fully, nothing detected
  if (D <= 4.0)        return "Unknown";
  if (D <= 8.0)        return "Alba";
  if (D <= 12.0)       return "C5";
  if (D <= 16.0)       return "C4";
  if (D <= 18.0)       return "C3/M5";
  if (D <= 21.0)       return "M4";
  if (D <= 25.0)       return "H1";
  if (D <= 32.0)       return "H2";
  if (D <= 38.0)       return "H3";
  return "Unknown";                            // D > 38mm
}

// ------------------------------------------------------------
// setup()
// ------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  SoftSerial.begin(115200);
  delay(500);

  // --- OLED init FIRST, so we can show status immediately ---
  Wire.begin(SDA_PIN, SCL_PIN);
  oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  if (!oledReady) {
    Serial.println("WARNING: OLED init failed! Continuing without display.");
  }

  oledMessage("Initializing", "","", 2);
  Serial.println("=== Cinnamon Grading Device ===");
  Serial.println("Initializing...");

  // --- Load cells ---
  Serial.println("Taring load cells - keep EMPTY...");
  delay(1500);   // brief pause so nothing is mid-motion/vibrating during tare
  loadCell1.begin();
  loadCell2.begin();
  loadCell1.setCalibrationFactor(LOADCELL1_CAL_FACTOR);
  loadCell2.setCalibrationFactor(LOADCELL2_CAL_FACTOR);
  loadCell1.setThreshold(LOADCELL1_THRESHOLD_G);
  loadCell2.setThreshold(LOADCELL2_THRESHOLD_G);

  // --- Jaw / motor ---
  jaw.begin(STEPS_BETWEEN_POSITIONS, RUN_CURRENT_MA);
  jaw.setSpeed(4500, 4500);
  jaw.setConditionCallback(jawTouchCondition);

  // --- Home: find the real closed position, then open ---
  Serial.println("Homing...");
  bool homed = false;
  while (!homed) {
    homed = jaw.home(600);
    if (!homed) {
      Serial.println("HOMING FAILED - retrying in 2s. Check load cell 2 / wiring.");
      oledMessage("Homing Error", "Retrying...","", 2);
      delay(2000);
    }
  }
  Serial.println("Homing complete.");

  jaw.openJaw();
  Serial.println("Jaw opened. Ready.");
}

// ------------------------------------------------------------
// loop()
// One full grading cycle per iteration, repeating forever.
// ------------------------------------------------------------
void loop() {
  // --- 1. Wait for a cinnamon stick to be placed ---
  oledMessage("Place the", "cinnamon","", 2);
  Serial.println("Waiting for cinnamon placement...");

  while (!loadCell1.load_on_cell()) {
    delay(100);   // small poll delay, avoids hammering the HX711 needlessly
  }

  Serial.println("Cinnamon detected on Load Cell 1.");

  // --- 2. Let it settle before taking the real weight reading ---
  delay(1000);
  float weight = loadCell1.load_value();
  Serial.print("Weight: ");
  Serial.print(weight, 3);
  Serial.println(" g");

  // --- 3. Close the jaw until it touches the cinnamon (grip sensor) ---
  oledMessage("Measuring...", "","", 2);
  long stepsLeft = jaw.checkClose();
  Serial.print("checkClose() steps left: ");
  Serial.println(stepsLeft);

  // --- 4. Convert steps-left into a diameter, then grade it ---
  float diameterMM = (float)stepsLeft / STEPS_PER_MM;
  String grade = gradeFromDiameter(diameterMM);

  Serial.print("Diameter: ");
  Serial.print(diameterMM, 2);
  Serial.print(" mm  ->  Grade: ");
  Serial.println(grade);

  // --- 5. Show the result ---
  oledMessage("Grade:" + grade,
              String(weight, 2) + "g " , String(diameterMM) + "mm",
              2);
  Serial.println("Result displayed.");
  delay(1000);

  // --- 6. Release and go back to waiting ---
  jaw.openJaw();
  Serial.println("Jaw opened. Cycle complete.\n");

  while (loadCell1.load_on_cell()) {
    delay(100);   // small poll delay, avoids hammering the HX711 needlessly
  }

  delay(1000);

  // Loop repeats automatically -> back to "Place the cinnamon"
}