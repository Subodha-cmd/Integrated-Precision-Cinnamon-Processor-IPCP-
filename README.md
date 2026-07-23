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
