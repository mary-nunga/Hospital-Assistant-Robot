# MediBot – Hospital Assistant Robot

MediBot is a prototype hospital assistant robot designed to support nurses by handling item delivery (medicine, files, supplies) and room sanitization through an automated spray module.
The goal is to reduce repetitive tasks, improve workflow efficiency, and strengthen hygiene practices in public hospitals.

## Project Summary

Hospitals in Kenya experience staff shortages, overcrowding, and heavy workloads. Tasks such as delivering items between departments and sanitizing rooms consume a lot of time. MediGo explores a low-cost robotic approach to automate these duties.

**The robot includes:**

-A wheeled chassis

-Ultrasonic obstacle detection

-A spray nozzle + pump module

-Raspberry Pi for command handling

-Arduino for motor and sensor control

-A simple Flask web interface for triggering delivery tasks

-3D-printed components for mounting and support

The system demonstrates the core idea of an affordable assistant robot suitable for local hospitals.

## System Overview
### 1. Arduino (Robot Control Layer)

-Controls DC motors for movement

-Reads ultrasonic sensor distances

-Handles spray pump + servo nozzle

-Reads tray sensor input

-Receives commands from the Raspberry Pi via serial

### 2. Raspberry Pi (Communication & Logic Layer)

-Runs the Flask web app

-Sends “start delivery” or “spray” commands to Arduino

-Receives status messages from the robot

-Acts as the bridge between the user and the hardware

### 3. Web App (User Layer)

-Simple dashboard where the user can trigger tasks

-Displays real-time status updates sent from the Pi

## 4. Simulation

Wokwi/Tinkercad simulation used for early testing of sensors, movement, and wiring before hardware assembly

## 5. Repository Structure
3D models/         → STL files used for 3D-printed robot parts

arduino/          → Arduino firmware for movement, sensing, and spraying

raspberrypi/      → Python scripts for communication and web app handling

MediGo/           → Flask web application files

photos/           → Build progress, wiring, and hardware setup images

simulation/       → Wokwi/Tinkercad simulation and test files

videos/           → Hardware test clips and demo recordings

README.md         → Project documentation

## 6. Demo

Screenshots and test clips are included in the photos/ and videos/ folders.

**Demo video link:** https://drive.google.com/file/d/1LzmaI2o7zKeHk_qB2Ba0mEwqdXBLG7g-/view?usp=drive_link

## Progress & Limitations
**What worked:**

-Individual hardware components (motors, sensors, servo, pump)

-Web app interface and Raspberry Pi server

-Serial communication tests

-Simulations for obstacle sensing and movement

-Full hardware assembly with 3D-printed parts

## Challenges during final testing:

-Motor movement was inconsistent during the last lab session

-Pump stopped responding after earlier tests

-Web app → Pi → Arduino communication became unstable

-Some real-time updates lagged or glitched

These issues are typical of hardware integration and can be resolved with more debugging time (especially power management and serial timing). The core architecture, however, is fully implemented.
