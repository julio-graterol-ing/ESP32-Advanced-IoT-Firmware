# 🚀 Advanced ESP32 IoT & Cloud Firmware Architecture

This repository contains my evolution into 32-bit dual-core architectures, focusing on industrial Internet of Things (IoT) engineering, asynchronous network stacks, and cloud event injection. 

> 📁 **Architectural Ancestry Note:** This codebase represents the direct architectural continuation of my fundamental 8-bit asynchronous firmware repository. The foundational concepts of bare-metal loops, hardware debouncing, and custom time-slicing metrics can be reviewed in the legacy repository: [Luz-led](https://github.com/julio-graterol-ing/Luz-led).

---

## 🛠️ 32-Bit Micro-Architectural Methodologies Deployed

* **Hardware Peripherals Abstraction (LEDC Engine):** Bypassing blocking software loops by leveraging independent silicon-level timers to drive hardware actuators.
* **Dual-Core Asynchronous Slicing:** Designing non-blocking background routines (`millis()`) compliant with the ESP32 background Wi-Fi and Bluetooth network stacks.
* **Enterprise Repository Governance:** Maintaining separate code repositories to isolate legacy 8-bit fundamental codebases from advanced connected 32-bit cloud deployments.
* **FreeRTOS Real-Time Task Allocation:** Pinned scheduling of network-heavy cryptographic operations onto specific physical silicon cores to guarantee hardware actuator priority.
* **Secure Compilation Variable Isolation:** Abstracting Wi-Fi credentials and cloud provider endpoints out of the compiler text segments to protect digital signatures.
* **Asynchronous Pub/Sub Event-Driven Steering:** Transitioning from heavy request-response paradigms to state-driven bidirectional MQTT sockets to decouple execution logic.

---

## 📊 Technical Milestones & Core Deployments

### 📦 Phase 2: Cloud Ingestion Platforms & Real-Time Multitasking Architecture (Latest Updates)

* **August 21, 2026 | FreeRTOS Message Queue Integration & Race Condition Elimination:**

  **Project: Thread-Safe Inter-Core Climate Data Queue Infrastructure.**  
  Successfully architected, compiled, and deployed a hardware-protected FreeRTOS message pipeline (`QueueManager`) to route environmental telemetry between distinct processing units.

  * *The Bottleneck:* Relying on shared global registers (`currentTemperature` and `currentHumidity`) created severe race condition risks in multi-core execution. When Core 1 mutated these variables during sensor sampling at the exact moment Core 0 attempted to parse them for MQTT publishing, the system risked memory corruption, data interleaving, or unhandled CPU exceptions.
  * *The Engineering Fix:* Eradicated global telemetry variables from the compilation scope. Engineered a modular message system that instantiates a structured `ClimateData` binary payload. The sensor subsystem on Core 1 captures data asynchronously, packages it, and executes a non-blocking hardware push (`xQueueSend`) with a 0ms wait threshold to preserve localized timing. On Core 0, the cloud task uses an optimized block window (`xQueueReceive`) to safely extract data, update safe local volatile caches for AJAX endpoints, and stream payloads to public networks, guaranteeing total thread-safe structural isolation.



* **August 18, 2026 | Bidirectional MQTT Broker Integration & Actuator Override:**

  **Project: Real-Time Event-Driven MQTT Telemetry Node and Remote Servo Steering.**  
  Successfully architected and deployed an asynchronous, low-overhead MQTT client integrated with the Adafruit IO cloud platform, establishing robust bidirectional pipelines for sensor data publishing and live actuator control.

  * *The Bottleneck:* Integrating the `Adafruit MQTT Library` introduced invasive transit dependecies (`WiFi101` and `WiFiNINA`) that triggered strict class redefinitions against the native ESP32 core, causing false syntax alerts and breaking hardware-level diagnostics. Additionally, typical inline string concatenation failed during dynamic feed construction since the preprocessor variables (`AIO_USERNAME`) are evaluated at runtime rather than compile-time. Mechanically, the local asynchronous kinematic loop continuously overwrote incoming remote data, snapping the actuator back to its auto-sweep instantly.
  * *The Engineering Fix:* Refactored the network engine layout by applying a strict `lib_ignore` filter in the environment core to isolate the native Espressif Wi-Fi stack. Implemented heap allocation via dynamic pointer instantiation (`new Adafruit_MQTT_Publish`) utilizing safe runtime buffer sizing (`snprintf`) to dynamically compile user paths. Isolated credential assets by routing all cloud variables exclusively through a segregated translation unit (`secrets.h` / `secrets.cpp`). Finally, engineered a thread-safe behavioral bypass utilizing an externalized status flag (`remoteControlActive`). The execution loop intercepts inbound packet callbacks on an ultra-low 10ms polling window, instantly disabling the automatic sweep to hand over deterministic, real-time positional control to cloud-side deslizadores without interrupting the local background server infrastructure.

* **August 15, 2026 | FreeRTOS Pinned Core TLS Webhook & Network Diagnostics:**

  **Project: Real-Time Dual-Core Cloud Webhook Injector with Integrated Connectivity Validation.**  
  Successfully architected and deployed a multi-threaded IoT network hub that evaluates climate threshold rules and asynchronously fires secure HTTPS payloads to the IFTTT cloud API.

  * *The Bottleneck:* Executing synchronous `HTTPClient` operations requiring high-overhead SSL/TLS handshakes (`mbedTLS`) caused severe stack fragmentation when combined with the local asynchronous HTTP server. The 16 KB default allocation overhead crashed volatile memory pools, resulting in runtime freezes (`start_ssl_client: -1`) and actuator jitter. Furthermore, external network environment restrictions (carrier tethering filtering) mimicked firmware-level socket blockages.
  * *The Engineering Fix:* Refactored the architecture by splitting logic compilation structures via `secrets.h` and separate translation units using `extern` pointers to eliminate linker variable duplication. Instantiated a native FreeRTOS worker kernel sequence (`xTaskCreatePinnedToCore`) pinning the cloud telemetry loop exclusively onto **Core 0** with a dedicated 16 KB stack buffer, leaving the master actuator timeline to run unhindered on Core 1. Implemented a bypass for strict Root CA chain validation via `client.setInsecure()` and increased handshake execution windows to 30s. Finally, integrated dual bare-metal network diagnostics—performing raw TCP handshakes to Google's public DNS (`8.8.8.8:53`) and raw TLS socket probes (`://ifttt.com`)—confirming real-time outbound packet ingestion and deterministic execution despite restrictive cellular networks.

### 📦 Phase 1: 32-Bit Silicon Cores & Hardware Peripherals

* **August 10, 2026 | Asynchronous AJAX Web Server & Servo Telemetry:**

  **Project: Dual-Endpoint Asynchronous HTTP Telemetry and Servo Visualizer.**  
  Successfully architected and deployed a non-blocking asynchronous HTTP web server on the Espressif ESP32 platform to stream environmental metrics and the real-time position of the servo motor over local Wi-Fi.

  * *The Engineering Fix:* Refactored the traditional page-reloading model by instantiating discrete HTTP Rest API endpoints (`/temperature`, `/humidity`, and the `/servo` endpoint). Injected an embedded JavaScript layout using asynchronous network request routines (`fetch()`) inside a PROGMEM string buffer. This allows the remote mobile or desktop client browser to poll the numeric metrics and the current servo angle directly from volatile registers every 2000ms for climate data and every 100ms for the motor position. The UI dynamically updates the specific Document Object Model (DOM) targets and moves a custom visual progress bar without redrawing the webpage canvas, keeping local SRAM overhead down to 13.5% and eliminating network lag.

* **August 8, 2026 | Asynchronous AJAX Web Server Telemetry Core:**

  **Project: Dual-Endpoint Asynchronous HTTP Telemetry API Node.**  
  Successfully architected and deployed a non-blocking asynchronous HTTP web server on the Espressif ESP32 platform to stream environmental metrics over local radio frequencies. The firmware completely separates data pipelines from visual rendering structures, optimizing memory consumption and bandwidth.

  * *The Engineering Fix:* Refactored the traditional page-reloading model by instantiating discrete HTTP Rest API endpoints (`/temperature` and `/humidity`). Injected an embedded JavaScript layout using asynchronous network request routines (`fetch()`) inside a PROGMEM string buffer. This allows the remote mobile or desktop client browser to poll numeric metrics directly from volatile registers every 2000ms. The UI dynamically patches the specific Document Object Model (DOM) container targets without redrawing the webpage canvas, reducing local SRAM overhead down to 13.3% and eliminating network link congestion.

* **August 4, 2026 | High-Speed LEDC PWM Engine Instantiation:**

  **Project: 16-Bit Register Servo Kinematic Sweep.**  
  Deployed my first native 32-bit firmware module to control a closed-loop servo actuator without relying on legacy 8-bit AVR architecture libraries. The implementation bypasses software timing constraints by establishing a hardware-driven interface directly over the chip's internal silicon oscillators.

  * *The Bottleneck:* Using the standard `Servo.h` library on the ESP32 causes critical compilation failure, as 8-bit timer registers do not exist in the Xtensa 32-bit dual-core architecture. 
  * *The Engineering Fix:* Designed a custom driver utilization pipeline over the ESP32 **LEDC (LED Control)** peripheral. Configured a dedicated hardware timer at a 50Hz frequency with an advanced 16-bit resolution map (scaling steps from 0 to 65535). Calibrated the discrete target pulse steps strictly between 1638 ticks (0 degrees) and 8192 ticks (180 degrees). The final asynchronous state engine executes continuous 180° kinematics over GPIO 18, ensuring smooth mechanical control with 0% CPU loop overhead.

---

## 📂 Project Structure

```text
├── include/
│   ├── CloudClient.h      # Outbound HTTPS network event signatures
│   ├── MqttClient.h       # Dynamic MQTT event routing signatures
│   ├── QueueManager.h     # Thread-safe FreeRTOS Queue infrastructure
│   ├── SensorRead.h       # Climate telemetry physical interfaces
│   ├── ServoControl.h     # Dynamic PWM angular drive interfaces
│   └── secrets.h          # Secured compilation definitions (gitignored)
├── src/
│   ├── CloudClient.cpp    # mbedTLS network engine implementation
│   ├── MqttClient.cpp     # Dynamic client & MQTT broker implementation
│   ├── QueueManager.cpp   # FreeRTOS hardware-protected queue execution
│   ├── SensorRead.cpp     # DHT11 sampling and binary payload queue dispatch
│   ├── ServoControl.cpp   # LEDC hardware clock driver instantiation
│   └── main.cpp           # Master 32-bit multi-core orchestrator
├── platformio.ini         # Espressif 32-bit dependency & environment core
└── README.md              # Active IoT engineering portfolio documentation

```

---
*Developed under professional firmware guidelines to demonstrate advanced embedded system competencies toward my career at Universidad Miguel Hernández (UMH).*
