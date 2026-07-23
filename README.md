# Integrated Precision Cinnamon Processor (IPCP)

[![PlatformIO](https://img.shields.io/badge/Framework-PlatformIO-orange.svg)](https://platformio.org/)
[![Microcontroller](https://img.shields.io/badge/MCU-ESP32--WROOM--32-blue.svg)](https://www.espressif.com/)
[![Standard](https://img.shields.io/badge/Compliance-SLS%2081%3A2021-green.svg)](https://www.slsi.lk/)

> **A Smart Electronic Cinnamon Grading System**  
> Developed at the Department of Electronic and Telecommunication Engineering, University of Moratuwa.

---

## 📌 Project Overview

Sri Lanka supplies 85–90% of the world's true cinnamon (*Cinnamomum verum*), yet a significant portion of its export value is lost due to subjective, manual grading. 

The **Integrated Precision Cinnamon Processor (IPCP)** is a feedback-driven electromechanical system governed by an ESP32 microcontroller. It replaces manual visual grading with precise, objective measurements of **quill diameter** and **mass**, categorizing cinnamon quills according to the **SLS 81:2021** standard (Alba, C, M, H grades).

### Key Features
* **Haptic Contact-Driven Caliper:** Utilizes a NEMA 17 stepper motor driving a rack-and-pinion jaw assembly. Physical contact is detected via a compression load cell and high-frequency interrupt, providing sub-millimeter digital diameter calculation.
* **Integrated Mass & Dimension Sensing:** Dual 24-bit HX711 ADCs concurrently capture quill mass and jaw displacement.
* **Silent & High-Precision Motion:** Driven by a TMC2208 motor driver in StealthChop mode to eliminate mechanical vibration noise that could distort load cell readings.
* **Real-time OLED Display:** Instant readout of the calculated DCD (Diameter/Class/Density) grade, exact diameter (mm), and weight (g).
* **40% Labor Reduction:** Streamlines post-harvest processing while providing non-subjective data for fair farm-gate pricing.

---

## 🛠️ System Architecture & Hardware  



+-------------------+
                  |   12V DC Supply   |
                  +---------+---------+
                            |
         +------------------+------------------+
         |                                     |
         v                                     v
 +---------------+                     +---------------+
 | Buck Converter|                     | TMC2208 Driver|
 +-------+-------+                     +-------+-------+
         | (Regulated 5V/3.3V)                 |
         v                                     v
 +---------------+                     +---------------+
 | ESP32-WROOM-32|<------------------->| NEMA 17 Stepper|
 +---+-------+---+                     +---------------+
     |       |
     |       +---> [ HX711 #1 ] <---> Platform Load Cell (Mass)
     |       +---> [ HX711 #2 ] <---> Compression Load Cell (Haptic Contact)
     |
     +-----------> [ OLED Display ]

### Hardware Specifications
| Parameter | Specification |
| :--- | :--- |
| **Microcontroller** | ESP32-WROOM-32 (Dual-Core) |
| **Motion Actuator** | NEMA 17 Stepper Motor with Rack & Pinion |
| **Motor Driver** | TMC2208 (StealthChop Silent Micro-stepping) |
| **Dimension Sensing** | Compression Load Cell + 24-bit HX711 ADC (Haptic trigger) |
| **Mass Sensing** | Platform Load Cell + 24-bit HX711 ADC |
| **Measurement Range**| 50 mm linear travel ($D = 50 - L$) |
| **Power Input** | 12 V DC (Onboard Buck Converter for logic levels) |
| **Grading Standard** | SLS 81:2021 |

---

## 📂 Repository Structure

This firmware is built using **[PlatformIO](https://platformio.org/)** for VS Code.

```text
├── .vscode/             # VS Code workspace configurations
├── include/             # Header files (.h)
├── lib/                 # Custom libraries and driver modules (HX711, Display, Motor)
├── src/                 # Source code (.cpp / main firmware routines)
├── test/                # Unit tests and hardware verification scripts
├── .gitignore           # Git ignore rules
└── platformio.ini       # PlatformIO project configuration & dependencies



🚀 Getting Started
Prerequisites
Install VS Code.

Install the PlatformIO IDE extension from the VS Code Marketplace.

Git clone this repository:


git clone [https://github.com/Subodha-cmd/IPCP-Firmware.git](https://github.com/Subodha-cmd/IPCP-Firmware.git)
cd IPCP-Firmware


Hardware Connections (Pinout Summary)PeripheralESP32 Pin / InterfaceNoteHX711 #1 (Mass)GPIO (DT / SCK)Platform weighing floorHX711 #2 (Contact)GPIO (DT / SCK)Interrupt-driven haptic stopTMC2208 DriverSTEP, DIR, ENConfigured for microsteppingOLED DisplayI2C (SDA, SCL)Real-time user feedback
