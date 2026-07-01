#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TMCStepper.h>
#include <AccelStepper.h>

// --- PIN DEFINITIONS ---
#define SW_RX            4
#define SW_TX            5
#define STEP_PIN         10
#define DIR_PIN          11


#define R_SENSE 0.11f      

SoftwareSerial SoftSerial(SW_RX, SW_TX);
TMC2208Stepper driver(&SoftSerial, R_SENSE);
AccelStepper stepper(1, STEP_PIN, DIR_PIN);

// A variable to keep track of our current mode
bool isStealthChop = true; 

void setup() {
  Serial.begin(9600);
  SoftSerial.begin(115200);

  // Initialize TMC2208 via UART
  driver.begin();
  driver.toff(5);                 
  driver.rms_current(735);        
  driver.microsteps(16);          
  
  // Turn on StealthChop (Quiet Mode) by default
  driver.en_spreadCycle(false); 
  driver.pwm_autoscale(true); // Required for StealthChop to work properly

  // Setup Movement
  stepper.setMaxSpeed(2000.0);     
  stepper.setAcceleration(1000.0); 

  // Print our menu to the Serial Monitor
  Serial.println("--- TMC2208 COMMAND CENTER ---");
  Serial.println("Send '1' to Toggle StealthChop / SpreadCycle");
  Serial.println("Send '2' to Print Live Diagnostics");
  Serial.println("------------------------------");
}

void loop() {
  // --- 1. KEEP THE MOTOR MOVING ---
  if (stepper.distanceToGo() == 0) {
    if (stepper.currentPosition() == 0) {
      stepper.moveTo(3200); 
    } else {
      stepper.moveTo(0);    
    }
  }
  stepper.run(); 

  // --- 2. LISTEN FOR YOUR COMMANDS ---
  // If you type something in the Serial Monitor...
  if (Serial.available() > 0) {
    char inChar = Serial.read(); // Read the character you typed
    
    // Feature 1: The Silence Test
    if (inChar == '1') {
      isStealthChop = !isStealthChop; // Flip the variable
      
      // driver.en_spreadCycle(false) = StealthChop (Quiet)
      // driver.en_spreadCycle(true)  = SpreadCycle (Loud/High Torque)
      driver.en_spreadCycle(!isStealthChop); 
      
      Serial.print("Mode changed to: ");
      Serial.println(isStealthChop ? "StealthChop (Ultra-Quiet)" : "SpreadCycle (Loud / High Torque)");
    } 
    
    // Feature 2: Live Diagnostics
    else if (inChar == '2') {
      Serial.println("\n--- Live Driver Health ---");
      
      // Check if StealthChop is actually engaged in the hardware
      Serial.print("StealthChop Active? ");
      Serial.println(driver.stealth() ? "YES" : "NO");

      // Check current draw scaling (0 to 31). 
      // This shows how hard the driver is working to push the motor.
      Serial.print("Current Scaling (0-31): ");
      Serial.println(driver.cs_actual());

      // Temperature Warnings (Crucial for 3D printers)
      Serial.print("Over-Temperature Pre-Warning? ");
      Serial.println(driver.otpw() ? "YES (It is getting hot!)" : "NO (Temps are good)");
      
      Serial.print("Over-Temperature Error? ");
      Serial.println(driver.ot() ? "YES (Shutting down!)" : "NO (All clear)");
      
      Serial.println("--------------------------\n");
    }
  }
}