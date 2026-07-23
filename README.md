# Integrated Precision Cinnamon Processor (IPCP)

**A Smart Electronic Cinnamon Grading System**

Department of Electronic and Telecommunication Engineering, University of Moratuwa

---

## Overview

Sri Lanka supplies 85–90% of the world's true cinnamon (*Cinnamomum verum*), yet an estimated 90% of it is exported in raw, ungraded quill form. Grading is still done almost entirely by manual visual inspection, a subjective process that varies with the grader's fatigue, experience, and lighting conditions — costing farmers and processors the price premium that objectively graded cinnamon commands.

**IPCP** is a low-cost, feedback-driven electromechanical instrument that objectively grades cinnamon quills against the **SLS 81:2021** standard by measuring **diameter** and **mass**, then classifying the quill into its DCD (Diameter/Class/Density) grade band (Alba, C, M, H) in real time.

A farmer places a single quill into the measurement channel → the system measures weight and diameter → the grade, weight, and diameter are displayed instantly on an onboard OLED screen.

## Repository Structure

```
.
├── .vscode/          # Editor configuration
├── include/           # Header files
├── lib/                # Project-specific libraries
├── src/                # Main firmware source (ESP32)
├── test/               # Unit / integration tests
├── .gitignore
└── platformio.ini      # PlatformIO project configuration
```

This is a [PlatformIO](https://platformio.org/) project targeting the **ESP32**.

## System Architecture

| Block | Component | Role |
|---|---|---|
| Microcontroller | ESP32 (dual-core) | Central controller — handles sensor interrupts and motor control simultaneously, IoT-ready |
| Motor Driver | TMC2208 (StealthChop) | Drives the stepper with silent, micro-stepped motion to avoid corrupting load-cell readings |
| Actuator | NEMA 17 stepper motor | Drives a rack-and-pinion jaw across a 50 mm measurement range |
| Weight Sensing | Load cell + HX711 (24-bit ADC) | Measures quill mass |
| Contact/Diameter Sensing | Compression load cell + HX711 (24-bit ADC) | Detects the moment of jaw contact via a high-frequency interrupt |
| Display | OLED | Shows grade, weight, and diameter |
| Power | 12 V DC supply + buck converter | Supplies the motor driver rail and regulated logic-level voltage |

### Diameter Measurement Principle

As the jaw advances toward the quill, the compression load cell detects contact and triggers a high-frequency interrupt via the HX711 ADC, halting the TMC2208 driver at the exact point of contact. Diameter is then computed from the stepper's recorded step count:

```
D = 50 - L
```

where `L` is the linear jaw travel (mm) over the 50 mm measurement range. Because this is a digital, step-count-based measurement rather than a noisy analog reading, it is inherently repeatable — validated over 10 repeated trials each across three grades (C4, C3, M4) with tight clustering and no drift.

## Hardware Specifications

| Parameter | Specification |
|---|---|
| Microcontroller | ESP32 (dual-core) |
| Motion actuator | NEMA 17 stepper motor |
| Motor driver | TMC2208 (StealthChop, silent micro-stepping) |
| Dimension sensing | HX711 + compression load cell (haptic contact detection) |
| Weight sensing | Independent platform load cell + HX711 |
| Measurement range | 50 mm linear jaw travel |
| Display | OLED |
| Power input | 12 V DC supply, stepped down via buck converter |
| Grading reference | SLS 81:2021 (Alba, C, M, H grade bands) |

## PCB

- 2-layer stack-up (top copper for signal routing, bottom copper as a ground plane) — a clean, continuous ground reference for the sensitive HX711 analog front-end while keeping fabrication cost low.
- Functional zones are kept physically separate on the board: dual HX711 load-cell interfaces, ESP32-WROOM-32 module with USB, buck-converter power stage, and the TMC2208 motor driver — isolating noisy switching/motor circuitry from the sensitive analog front-end.
- Designed in Altium Designer.

## Why a Mechanical Approach (Not Optical)

An earlier design explored non-contact optical shadow-profiling (a CCD sensor reading a quill's shadow width). It was rejected because:

- **Dust interference** — organic dust in processing sheds settles on lenses and creates false shadows.
- **Bark profile bias** — optical systems measure surface fuzz/loose fibre rather than structural diameter.
- **Non-integrated sensing** — a CCD approach can't weigh the quill, requiring a redundant second measurement step.

The load-cell-based mechanical design measures both diameter and mass with a single integrated mechanism and is materially more robust to dust, vibration, and humidity.

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/install) (VS Code extension or CLI)
- ESP32 dev board
- Hardware assembly per the schematic in `hardware/` (or as described in the final report)

### Build & Upload

```bash
# Clone the repository
git clone <repo-url>
cd <repo-name>

# Build
pio run

# Upload to ESP32
pio run --target upload

# Open serial monitor
pio device monitor
```

### Calibration

On first use, the weighing load cell and the jaw's mechanical zero-point must be calibrated. Follow the on-device calibration procedure (system-zero routine) before taking grading measurements.

## Future Improvements

- **Moisture sensing** — SLS 81:2021 also specifies a ≤14% moisture threshold for quills; the current system grades on diameter and mass only.
- **IoT data logging** — cloud dashboard for cooperatives/exporters to track grading throughput and quality trends.
- **Automated sorting** — auto-sort graded quills into separate bins.
- **Battery-powered field variant** — for use away from a fixed 12 V supply.
- **Enclosure ruggedisation** — improved dust/humidity sealing for long-term shed deployment.

## Team

| Member | Contribution |
|---|---|
| Ransara Maldeniya | Testing, Assembly, Coding, Documentation, Debugging |
| Nayanajith Ranasinghe | Enclosure Design, Business Planning, Assembly, Debugging |
| Subodha Peiris | Testing, Assembly, Schematic Design, PCB Soldering, Coding, Debugging |
| Dulen Sandiw | Assembly, Enclosure Design, Debugging |
| Rasula Geesara | PCB Design, PCB Assembly, PCB Soldering, Debugging |

## Reference Standard

Sri Lanka Standards Institute — **SLS 81:2021**, Cinnamon Quill Specification.

## License

*(Add your chosen license here, e.g. MIT, Apache 2.0.)*
