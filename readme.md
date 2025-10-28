# Switched Mode Power Converter Simulator/Emulator

## Overview

This project is a **Switched Mode Power Converter Simulator/Emulator**, developed as part of the course **BL40A1101 ESP** at **LUT School of Energy Systems, Electrical Engineering**. The goal is to construct a **DC-converter controller** and a **converter model** that can be controlled by your controller in real-time. The project also includes optional **bonus tasks** for extra points.

The work can be executed on **Zynq** or **Nucleo boards** and requires only a board and a computer. Some external components (LEDs, switches) may be needed depending on the board.

## Project members

Juhani Manninen
Leevi Lahtinen
Saska Kynäslahti
Tuure

---

## Assignment Details

* **First due date:** 31.1.2025
* **Final submission date:** 30.4.2025
* **Submission:** Moodle, including source code, short description, and optionally a video demonstrating the program.

### Evaluation

| Feature           | Minimum (50–65 pts)              | Fair (66–80 pts)                                 | Good (>80 pts)                                                    |
| ----------------- | -------------------------------- | ------------------------------------------------ | ----------------------------------------------------------------- |
| Functionality     | Control and model works (almost) | Works fairly, allows parameter/reference changes | Works well, handles state changes properly                        |
| Code Style        | Readable                         | Well commented                                   | Well commented & self-explanatory                                 |
| Program Structure | Working code                     | Structured parts                                 | Modular code, information hiding, interface functions             |
| Scheduler         | Polling with interrupts          | Polling with interrupts + some scheduling        | Scheduled tasks (own or FreeRTOS), systematic & prioritized       |
| Controller        | Basic PI                         | Proper structure                                 | Reentrant, extra features (anti-winding, derivative filters)      |
| User Interface    | Buttons & LEDs                   | Fair console/menu                                | Good menu structure, responsive buttons/console, data protection  |
| Report            | Code only + description          | + Short video                                    | + Video showing board + running code, audio/subtitles recommended |

**Bonus tasks (0–10 points each, max 2 graded = 0–20 pts):**

1. Single-phase inverter simulation
2. Feed PWM modulated signal to the model
3. MQTT communication to a database (Zynq only)
4. OPC UA communication to a database (Zynq only)

---

## Features

### Real-Time Program Structure

* Implemented using:

  * **Polling with interrupts**,
  * **Own scheduler**, or
  * **FreeRTOS tasks** (Zynq: only tasks, no custom interrupts; Nucleo: tasks + interrupts possible).
* Avoid `sleep` or blocking loops outside initialization.

### Converter Model

* Discrete **state-space model** of a DC-converter (continuous-time model given in the assignment).
* Output: voltage ( u_3 ).
* Discretization performed using MATLAB `c2d` with a 50 kHz sampling rate.

### Controller

* **PI controller** (P & I terms configurable).
* Controller output drives **PWM output** and LED brightness for visualization.
* Parameters configurable via **buttons** or **UART console**.

### User Interface

* **Buttons:** Mode selection, parameter increment/decrement.
* **LEDs:** Mode indicators.
* **UART Console:** Mode changes, parameter values, reference voltage.
* **Semaphore/Protection:** Ensures safe interaction between buttons and UART.

---

## Bonus Features

1. **Inverter Simulation:** Single-phase 50 Hz output.
2. **PWM Input to Simulation:** H-bridge simulation with PI-controlled PWM signal.
3. **MQTT to Database (Zynq Only):** Sends data from board to database via MQTT.
4. **OPC UA to Database (Zynq Only):** Sends data from board to database via OPC UA.

*Videos demonstrating these features are recommended for full bonus evaluation.*

---


## Submission

* Submit **source code**, **short description**, and optionally a **video** demonstrating the program.
* Ensure your video clearly shows the board, running code, and user interactions.
* Follow the submission deadlines on Moodle.

---

