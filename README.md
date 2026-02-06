# Cardputer‑ADV Motion Lab

A collection of interactive IMU‑driven demos for the M5Stack Cardputer‑ADV.  
Tilt, rotate, and shake the device to explore real‑time motion visualizations, mini‑games, and diagnostic tools.

---

## Features

### Splash Screen
- Centered title and subtitle  
- Auto‑advance after 3 seconds or keypress

### Menu System
Keyboard or button navigation:
- `1` — 3D Cube  
- `2` — Bubble Level  
- `3` — Tilt Game  
- `4` — IMU Graph  
- `5` — Raw Viewer  
- Buttons A/B/C map to the first three demos

### 3D Cube Demo
- IMU‑controlled 3D wireframe cube  
- Pitch/roll rotation from accelerometer  
- Depth‑based shading  
- Smooth ~60 FPS rendering

### Bubble Level
- Circular spirit level with crosshair grid  
- Neon bubble with natural inverted‑axis movement

### Tilt Game
- Move a player dot by tilting the device  
- Randomized goal target  
- Collision detection with flash feedback

### IMU Graph (Oscilloscope Mode)
- Scrolling graph of accelerometer and gyroscope data  
- Circular buffer for smooth motion  
- Distinct color‑coded traces

### Raw Data Viewer
- Live accelerometer and gyro values  
- Computed pitch and roll  
- Clean text layout for diagnostics

---

## Project Structure

Cardputer-ADV-Motion-Lab/
│
├── MotionLab.ino                # Main firmware
├── README.md                        # Project documentation
└── assets/              # (Optional) screenshots, diagrams

Code

---

## Requirements

- M5Stack Cardputer‑ADV  
- Arduino IDE or PlatformIO  
- Libraries:
  - M5Unified  
  - M5Cardputer  

---

## Getting Started

1. Install Arduino IDE and ESP32 board support  
2. Install **M5Unified** and **M5Cardputer** libraries  
3. Open the `.ino` file  
4. Select the Cardputer‑ADV board profile  
5. Flash the firmware to the device

---

## Controls

| Action        | Input                     |
|---------------|----------------------------|
| Exit demo     | Button A or ESC key        |
| Select mode   | Keys `1`–`5`               |
| Tilt control  | Physical device movement   |

---

## Modes Overview

| Mode   | Description                        |
|--------|------------------------------------|
| Splash | Intro screen                       |
| Menu   | Mode selector                      |
| Cube   | IMU‑rotated 3D cube                |
| Level  | Bubble level                       |
| Game   | Tilt‑controlled mini‑game          |
| Graph  | Scrolling IMU oscilloscope         |
| Raw    | Raw IMU + orientation values       |

---

## Known Issues / To‑Fix List

- Occasional flicker when switching modes  
- Cube shading uses static Z instead of rotated Z  
- Graph scaling could be adaptive  
- Menu could use a highlight cursor  
- Keyboard polling may miss rapid keypresses  
- Exiting demos is immediate; could use a confirmation overlay  

---

## Contributing

Pull requests are welcome.  
Areas especially open for improvement:
- IMU filtering (Kalman, complementary filter)  
- UI polish and animations  
- Additional motion demos  
- Performance optimizations  

---

## License

MIT License (recommended, but choose what fits your project).
