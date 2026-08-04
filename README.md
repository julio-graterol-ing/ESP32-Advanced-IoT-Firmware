# 🚀 Advanced ESP32 IoT & Cloud Firmware Architecture

This repository contains my evolution into 32-bit dual-core architectures, focusing on industrial Internet of Things (IoT) engineering, asynchronous network stacks, and cloud event injection. 

> 📁 **Architectural Ancestry Note:** This codebase represents the direct architectural continuation of my fundamental 8-bit asynchronous firmware repository. The foundational concepts of bare-metal loops, hardware debouncing, and custom time-slicing metrics can be reviewed in the legacy repository: [Luz-led](https://github.com/julio-graterol-ing/Luz-led).

---

## 🛠️ 32-Bit Micro-Architectural Methodologies Deployed

* **Hardware Peripherals Abstraction (LEDC Engine):** Bypassing blocking software loops by leveraging independent silicon-level timers to drive hardware actuators.
* **Dual-Core Asynchronous Slicing:** Designing non-blocking background routines (`millis()`) compliant with the ESP32 background Wi-Fi and Bluetooth network stacks.
* **Enterprise Repository Governance:** Maintaining separate code repositories to isolate legacy 8-bit fundamental codebases from advanced connected 32-bit cloud deployments.

---

## 📊 Technical Milestones & Core Deployments

### 📦 Phase 1: 32-Bit Silicon Cores & Hardware Peripherals (Latest Updates)

* **August 4, 2026 | High-Speed LEDC PWM Engine Instantiation:**

  **Project: 16-Bit Register Servo Kinematic Sweep.**  
  Deployed my first native 32-bit firmware module to control a closed-loop servo actuator without relying on legacy 8-bit AVR architecture libraries. The implementation bypasses software timing constraints by establishing a hardware-driven interface directly over the chip's internal silicon oscillators.

  * *The Bottleneck:* Using the standard `Servo.h` library on the ESP32 causes critical compilation failure, as 8-bit timer registers do not exist in the Xtensa 32-bit dual-core architecture. 
  * *The Engineering Fix:* Designed a custom driver utilization pipeline over the ESP32 **LEDC (LED Control)** peripheral. Configured a dedicated hardware timer at a 50Hz frequency with an advanced 16-bit resolution map (scaling steps from 0 to 65535). Calibrated the discrete target pulse steps strictly between 1638 ticks (0 degrees) and 8192 ticks (180 degrees). The final asynchronous state engine executes continuous 180° kinematics over GPIO 18, ensuring smooth mechanical control with 0% CPU loop overhead.

---

## 📂 Project Structure

```text
├── src/
│   └── main.cpp           # Master 32-bit peripheral execution scheduler
├── platformio.ini         # Espressif 32-bit dependency & environment core
└── README.md              # Active IoT engineering portfolio documentation
```

---
*Developed under professional firmware guidelines to demonstrate advanced embedded system competencies toward my career at Universidad Miguel Hernández (UMH).*

