# Cardputer‑ADV Motion Lab

An interactive IMU‑driven demo suite for the **M5Stack Cardputer‑ADV**, showcasing real‑time motion graphics, tilt‑based controls, and diagnostic tools.  
This project is designed as a hands‑on playground for exploring the Cardputer‑ADV’s accelerometer and gyroscope.

---

## Features

### 3D Cube (IMU‑Driven, Live Zoom)
- Real‑time pitch/roll rotation from accelerometer  
- Smooth ~60 FPS rendering  
- **Live zoom control** using `+` and `-` keys  
- Reset zoom with `0`  

### Bubble Level
- Circular spirit level  
- Neon bubble with natural inverted‑axis movement  
- Auto‑scaled circle to fit the ADV screen  

### Tilt Game
- Move a player dot by tilting the device  
- Randomized goal target  
- Collision flash effect
- Alternating control styles
- Smooth motion and clamped boundaries  

### IMU Graph (Oscilloscope Mode)
- Scrolling graph of accelerometer + gyro data  
- Circular buffer for smooth scrolling  
- Six color‑coded traces  
- Great for debugging IMU noise or drift  

### Raw Data Viewer
- Live accelerometer and gyro values  
- Computed pitch and roll  
- Compact text layout for the ADV’s 135px height  

### Unified Exit Controls
All demos can be exited using: 
- **Del**

![Screen Image](https://github.com/Mraanderson/CardputerADV-MotionLab/blob/main/docs/ArduinoIDE/gforce.jpg)
---

## Controls

| Action        | Input                          |
|---------------|---------------------------------|
| Exit demo     | Btn A, ESC, Backspace, Delete   |
| Menu select   | Keys `1`–`5`                    |
| Cube zoom     | `+` / `-` / `0`                 |
| Tilt control  | Physical device movement        |

---

## Requirements

- M5Stack **Cardputer‑ADV**
- Arduino IDE or PlatformIO
- Libraries:
  - `M5Unified`
  - `M5Cardputer`

---

## Getting Started

1. Install Visual Code Studio and the PlatformIO extension
2. Create a new PlatformIO project
3. Select the **M5stack StampS3** as the board
4. Copy the .ini and src/main.cpp contents over the template files
5. Connect Cardputer ADV by USB Data cable
6. Hold down **Go** and reset to put into flashing mode
7. Upload and optionally Build, to use your own firmware.bin file

Or flash the .bin file from releases - Web flasher coming soon...

---

## Modes Overview

| Mode   | Description                        |
|--------|------------------------------------|
| Cube   | IMU‑rotated 3D cube + live zoom    |
| Level  | Bubble level                       |
| Game   | Tilt‑controlled mini‑game          |
| G-Force  | G-Force measure and peak test         |
| Graph  | Scrolling IMU oscilloscope         |
| Raw    | Raw IMU + orientation values       |

---

## Known Issues / Future Improvements

- Menu could benefit from a highlight cursor  
- IMU graph scaling could be adaptive  
- Cube could optionally support yaw via gyro  
- Add optional smoothing filters (Kalman / complementary)  
- Add more demos (compass, horizon, 3D ball, etc.)  

---

## License

MIT License
