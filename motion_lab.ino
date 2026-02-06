// ================================================================
//  Cardputer ADV Motion Lab
//  Core Setup, Globals, Theme, IMU Init
// ================================================================

#include <M5Unified.h>
#include <cmath>
#include <M5Cardputer.h>

// Projected 2D point
struct P2 {
  int x, y;
};

// 3D point
struct P3 {
  float x, y, z;
};

// -------------------------------
//  Colour Theme (Menu + Splash)
// -------------------------------
static const uint16_t COL_BG      = BLACK;
static const uint16_t COL_TEXT    = GREEN;
static const uint16_t COL_HILIGHT = GREEN;

// -------------------------------
//  App Modes
// -------------------------------
enum AppMode {
  MODE_SPLASH,
  MODE_MENU,
  MODE_CUBE,
  MODE_LEVEL,
  MODE_GAME,
  MODE_GRAPH,
  MODE_RAW
};

AppMode currentMode = MODE_SPLASH;

// -------------------------------
//  IMU Variables
// -------------------------------
float ax, ay, az;
float gx, gy, gz;
float pitch, roll, yaw;

// -------------------------------
//  Cube zoom (live adjustable)
// -------------------------------
float zoomScale = 90.0f;   // initial cube size (bigger than 60)

// -------------------------------
//  Forward Declarations
// -------------------------------
void drawSplash();
void drawMenu();
void demoCube();
void demoLevel();
void demoTiltGame();
void demoGraph();
void demoRaw();

// ================================================================
//  Utility: unified exit key handling
// ================================================================
bool exitRequested() {
  // BtnA, ESC (0x29), Backspace (0x2A), Delete (0x4C)
  return M5.BtnA.wasPressed() ||
         M5Cardputer.Keyboard.isKeyPressed(0x29) ||
         M5Cardputer.Keyboard.isKeyPressed(0x2A) ||
         M5Cardputer.Keyboard.isKeyPressed(0x4C);
}

// ================================================================
//  Setup
// ================================================================
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5Cardputer.begin();

  M5.Display.setRotation(1);
  M5.Display.setTextSize(2);
  M5.Display.setTextColor(COL_TEXT, COL_BG);
  M5.Display.fillScreen(COL_BG);

  // IMU Init
  if (!M5.Imu.begin()) {
    M5.Display.setCursor(0, 0);
    M5.Display.println("IMU init failed!");
    while (true) delay(100);
  }

  currentMode = MODE_SPLASH;
}

// ================================================================
//  Main Loop
// ================================================================
void loop() {
  M5.update();
  M5Cardputer.update();

  switch (currentMode) {
    case MODE_SPLASH: drawSplash();   break;
    case MODE_MENU:   drawMenu();     break;
    case MODE_CUBE:   demoCube();     break;
    case MODE_LEVEL:  demoLevel();    break;
    case MODE_GAME:   demoTiltGame(); break;
    case MODE_GRAPH:  demoGraph();    break;
    case MODE_RAW:    demoRaw();      break;
  }
}

// ================================================================
//  Splash Screen (Large Title + Smaller Subtitle)
// ================================================================
void drawSplash() {
  static bool firstRun = true;
  static uint32_t startTime = 0;

  if (firstRun) {
    firstRun = false;
    startTime = millis();

    M5.Display.fillScreen(COL_BG);
    M5.Display.setTextColor(COL_TEXT, COL_BG);

    // Large title
    M5.Display.setTextSize(2);
    int16_t tw = M5.Display.textWidth("Cardputer ADV");
    M5.Display.setCursor((M5.Display.width() - tw) / 2, 30);
    M5.Display.print("Cardputer ADV");

    tw = M5.Display.textWidth("Motion Lab");
    M5.Display.setCursor((M5.Display.width() - tw) / 2, 60);
    M5.Display.print("Motion Lab");

    // Subtitle
    M5.Display.setTextSize(1);
    tw = M5.Display.textWidth("Tilt, rotate, shake — explore the IMU!");
    M5.Display.setCursor((M5.Display.width() - tw) / 2, 100);
    M5.Display.print("Tilt, rotate, shake — explore the IMU!\n");
    M5.Display.print("\n v0.03");
  }

  // Hold for 3 seconds or exit on key press
  if (millis() - startTime > 3000 ||
      M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
  }
}

// ================================================================
//  Menu Screen
// ================================================================
void drawMenu() {
  static bool first = true;

  if (first) {
    first = false;

    M5.Display.fillScreen(COL_BG);
    M5.Display.setTextColor(COL_TEXT, COL_BG);
    M5.Display.setTextSize(2);

    int y = 20;
    M5.Display.setCursor(10, y);
    M5.Display.print("IMU Demo Menu");
    y += 30;

    M5.Display.setTextSize(1);
    M5.Display.setCursor(10, y);  M5.Display.print("1. 3D Cube");       y += 15;
    M5.Display.setCursor(10, y);  M5.Display.print("2. Bubble Level");  y += 15;
    M5.Display.setCursor(10, y);  M5.Display.print("3. Tilt Game");     y += 15;
    M5.Display.setCursor(10, y);  M5.Display.print("4. IMU Graph");     y += 15;
    M5.Display.setCursor(10, y);  M5.Display.print("5. Raw Viewer");
  }

  // Only poll input — no drawing
  if (M5Cardputer.Keyboard.isKeyPressed('1')) currentMode = MODE_CUBE;
  if (M5Cardputer.Keyboard.isKeyPressed('2')) currentMode = MODE_LEVEL;
  if (M5Cardputer.Keyboard.isKeyPressed('3')) currentMode = MODE_GAME;
  if (M5Cardputer.Keyboard.isKeyPressed('4')) currentMode = MODE_GRAPH;
  if (M5Cardputer.Keyboard.isKeyPressed('5')) currentMode = MODE_RAW;

  if (M5.BtnA.wasPressed()) currentMode = MODE_CUBE;
  if (M5.BtnB.wasPressed()) currentMode = MODE_LEVEL;
  if (M5.BtnC.wasPressed()) currentMode = MODE_GAME;

  // Reset first-run flag when leaving menu
  if (currentMode != MODE_MENU) first = true;
}

// ================================================================
//  3D Cube Demo (Shaded, IMU‑Driven)
// ================================================================

// Cube vertices
P3 cubeVerts[8] = {
  {-1, -1, -1},
  { 1, -1, -1},
  { 1,  1, -1},
  {-1,  1, -1},
  {-1, -1,  1},
  { 1, -1,  1},
  { 1,  1,  1},
  {-1,  1,  1}
};

// Cube edges
int cubeEdges[12][2] = {
  {0,1},{1,2},{2,3},{3,0},
  {4,5},{5,6},{6,7},{7,4},
  {0,4},{1,5},{2,6},{3,7}
};

// Simple projection (uses live zoomScale)
P2 projectPoint(P3 p) {
  float scale   = zoomScale;
  float zOffset = 4.0f;
  float inv     = 1.0f / (p.z + zOffset);

  return {
    (int)(p.x * scale * inv + M5.Display.width()  / 2),
    (int)(p.y * scale * inv + M5.Display.height() / 2)
  };
}

// Rotate a point by pitch/roll/yaw
P3 rotatePoint(P3 v, float pitch, float roll, float yaw) {
  float cp = cos(pitch), sp = sin(pitch);
  float cr = cos(roll),  sr = sin(roll);
  float cy = cos(yaw),   sy = sin(yaw);

  // Yaw
  float x1 = v.x * cy - v.z * sy;
  float z1 = v.x * sy + v.z * cy;

  // Pitch
  float y2 = v.y * cp - z1 * sp;
  float z2 = v.y * sp + z1 * cp;

  // Roll
  float x3 = x1 * cr - y2 * sr;
  float y3 = x1 * sr + y2 * cr;

  return {x3, y3, z2};
}

void demoCube() {
  M5.update();

  // Exit back to menu
  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  // Zoom controls (+ / - / 0)
  if (M5Cardputer.Keyboard.isKeyPressed('+') ||
      M5Cardputer.Keyboard.isKeyPressed('=')) {   // '=' often shares key with '+'
    zoomScale += 2.0f;
    if (zoomScale > 140.0f) zoomScale = 140.0f;
  }

  if (M5Cardputer.Keyboard.isKeyPressed('-') ||
      M5Cardputer.Keyboard.isKeyPressed('_')) {
    zoomScale -= 2.0f;
    if (zoomScale < 20.0f) zoomScale = 20.0f;
  }

  if (M5Cardputer.Keyboard.isKeyPressed('0')) {
    zoomScale = 90.0f;  // reset to default
  }

  // Read IMU
  M5.Imu.getAccel(&ax, &ay, &az);

  // Compute tilt angles from accelerometer
  pitch = atan2(ay, az) * 57.2958f;
  roll  = atan2(-ax, sqrtf(ay*ay + az*az)) * 57.2958f;
  yaw   = 0;

  // Convert degrees → radians
  float p = pitch * 0.0174533f;
  float r = roll  * 0.0174533f;
  float y = yaw   * 0.0174533f;

  // Clear screen
  M5.Display.fillScreen(BLACK);

  // Rotate + project all vertices
  P2 pts[8];
  P3 rpVerts[8];
  for (int i = 0; i < 8; i++) {
    rpVerts[i] = rotatePoint(cubeVerts[i], p, r, y);
    pts[i]     = projectPoint(rpVerts[i]);
  }

  // Draw shaded edges (based on rotated Z)
  for (int i = 0; i < 12; i++) {
    int a = cubeEdges[i][0];
    int b = cubeEdges[i][1];

    float shade = (rpVerts[a].z + rpVerts[b].z + 2.0f) * 0.25f;
    shade = constrain(shade, 0.2f, 1.0f);

    uint8_t  intensity = (uint8_t)(shade * 255);
    uint16_t col       = M5.Display.color565(0, intensity, intensity); // cyan-ish shading

    M5.Display.drawLine(pts[a].x, pts[a].y, pts[b].x, pts[b].y, col);
  }

  delay(16); // ~60 FPS
}


// ================================================================
//  Bubble Level Demo
// ================================================================
void demoLevel() {
  M5.update();

  // Exit back to menu
  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  // Read accelerometer
  M5.Imu.getAccel(&ax, &ay, &az);

  // Screen center
  int cx = M5.Display.width()  / 2;
  int cy = M5.Display.height() / 2;

  // Bubble movement scaling
  float scale = 80.0f;

  // Bubble position (invert axes for natural feel)
  int bx = cx + (int)(ax * scale);
  int by = cy + (int)(-ay * scale);

  // Clamp bubble to screen
  bx = constrain(bx, 10, M5.Display.width()  - 10);
  by = constrain(by, 10, M5.Display.height() - 10);

  // Draw background
  M5.Display.fillScreen(BLACK);

  // Draw grid
  uint16_t gridCol = M5.Display.color565(0, 180, 255);
  M5.Display.drawLine(cx, 0, cx, M5.Display.height(), gridCol);
  M5.Display.drawLine(0, cy, M5.Display.width(), cy, gridCol);

  // Outer circle (fit to screen)
  int radius = min(cx, cy) - 5;
  M5.Display.drawCircle(cx, cy, radius, gridCol);

  // Bubble (neon yellow)
  uint16_t bubbleCol = M5.Display.color565(255, 255, 0);
  M5.Display.fillCircle(bx, by, 12, bubbleCol);

  delay(16); // ~60 FPS
}

// ================================================================
//  Tilt Game Demo
// ================================================================
void demoTiltGame() {
  M5.update();

  // Exit back to menu
  static bool firstRun = true;
  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    firstRun = true;
    return;
  }

  // Read accelerometer
  M5.Imu.getAccel(&ax, &ay, &az);

  // Game area
  int w = M5.Display.width();
  int h = M5.Display.height();

  // Player position (static so it persists frame-to-frame)
  static float px;
  static float py;

  if (firstRun) {
    px = w / 2.0f;
    py = h / 2.0f;
    firstRun = false;
  }

  // Sensitivity
  float speed = 4.0f;

  // Update position (invert axes for natural feel)
  px += ax * speed;
  py += -ay * speed;

  // Clamp to screen
  if (px < 10)      px = 10;
  if (px > w - 10)  px = w - 10;
  if (py < 10)      py = 10;
  if (py > h - 10)  py = h - 10;

  // Draw background
  M5.Display.fillScreen(BLACK);

  // Draw border
  uint16_t borderCol = M5.Display.color565(0, 200, 255);
  M5.Display.drawRect(5, 5, w - 10, h - 10, borderCol);

  // Draw player (bright magenta)
  uint16_t playerCol = M5.Display.color565(255, 0, 200);
  M5.Display.fillCircle((int)px, (int)py, 8, playerCol);

  // Optional: small goal target
  static int gx;
  static int gy;
  static bool goalInit = false;
  if (!goalInit) {
    gx = w / 4;
    gy = h / 3;
    goalInit = true;
  }

  uint16_t goalCol = M5.Display.color565(0, 255, 0);
  M5.Display.fillCircle(gx, gy, 6, goalCol);

  // Collision check
  int dx = (int)px - gx;
  int dy = (int)py - gy;
  if (dx * dx + dy * dy < 14 * 14) {
    // Reset goal to a new random spot
    gx = 20 + (rand() % (w - 40));
    gy = 20 + (rand() % (h - 40));

    // Flash effect
    M5.Display.fillScreen(M5.Display.color565(255, 255, 0));
    delay(80);
  }

  delay(16); // ~60 FPS
}

// ================================================================
//  IMU Graph Demo (Scrolling Oscilloscope)
// ================================================================

// Graph buffer width (scrolling)
static const int GRAPH_W = 240;
static const int GRAPH_H = 135;

// Circular index
static int graphIndex = 0;

// Buffers for accel + gyro
static float bufAx[GRAPH_W];
static float bufAy[GRAPH_W];
static float bufAz[GRAPH_W];

static float bufGx[GRAPH_W];
static float bufGy[GRAPH_W];
static float bufGz[GRAPH_W];

void demoGraph() {
  M5.update();

  // Exit back to menu
  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  // Read IMU
  M5.Imu.getAccel(&ax, &ay, &az);
  M5.Imu.getGyro(&gx, &gy, &gz);

  // Store into circular buffers
  bufAx[graphIndex] = ax;
  bufAy[graphIndex] = ay;
  bufAz[graphIndex] = az;

  bufGx[graphIndex] = gx * 0.05f;  // scaled down for visibility
  bufGy[graphIndex] = gy * 0.05f;
  bufGz[graphIndex] = gz * 0.05f;

  graphIndex = (graphIndex + 1) % GRAPH_W;

  // Clear screen
  M5.Display.fillScreen(BLACK);

  // Draw midline
  uint16_t midCol = M5.Display.color565(80, 80, 80);
  M5.Display.drawLine(0, GRAPH_H / 2, GRAPH_W, GRAPH_H / 2, midCol);

  // Draw traces
  for (int i = 0; i < GRAPH_W - 1; i++) {
    int idx1 = (graphIndex + i)     % GRAPH_W;
    int idx2 = (graphIndex + i + 1) % GRAPH_W;

    auto mapY = [&](float v) {
      return (int)(GRAPH_H / 2 - v * 20.0f);
    };

    // Accel colours
    uint16_t colAx = M5.Display.color565(255, 0,   0);   // red
    uint16_t colAy = M5.Display.color565(0,   255, 0);   // green
    uint16_t colAz = M5.Display.color565(0,   128, 255); // blue

    // Gyro colours
    uint16_t colGx = M5.Display.color565(255, 255, 0);   // yellow
    uint16_t colGy = M5.Display.color565(255, 0,   255); // magenta
    uint16_t colGz = M5.Display.color565(0,   255, 255); // cyan

    // Draw accel
    M5.Display.drawLine(i, mapY(bufAx[idx1]), i + 1, mapY(bufAx[idx2]), colAx);
    M5.Display.drawLine(i, mapY(bufAy[idx1]), i + 1, mapY(bufAy[idx2]), colAy);
    M5.Display.drawLine(i, mapY(bufAz[idx1]), i + 1, mapY(bufAz[idx2]), colAz);

    // Draw gyro
    M5.Display.drawLine(i, mapY(bufGx[idx1]), i + 1, mapY(bufGx[idx2]), colGx);
    M5.Display.drawLine(i, mapY(bufGy[idx1]), i + 1, mapY(bufGy[idx2]), colGy);
    M5.Display.drawLine(i, mapY(bufGz[idx1]), i + 1, mapY(bufGz[idx2]), colGz);
  }

  delay(16); // ~60 FPS
}

// ================================================================
//  Raw Data Viewer
// ================================================================
void demoRaw() {
  M5.update();

  // Exit back to menu
  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  // Read IMU
  M5.Imu.getAccel(&ax, &ay, &az);
  M5.Imu.getGyro(&gx, &gy, &gz);

  // Compute tilt angles from accelerometer
  pitch = atan2(ay, az) * 57.2958f;
  roll  = atan2(-ax, sqrtf(ay*ay + az*az)) * 57.2958f;
  yaw   = 0;

  // Clear screen
  M5.Display.fillScreen(COL_BG);
  M5.Display.setTextColor(COL_TEXT, COL_BG);
  M5.Display.setTextSize(1);  // smaller to fit all lines

  int y = 5;

  M5.Display.setCursor(5, y);
  M5.Display.print("Raw IMU Data");
  y += 15;

  // Accelerometer
  M5.Display.setCursor(5, y);
  M5.Display.printf("Accel X: %.2f", ax); y += 12;
  M5.Display.setCursor(5, y);
  M5.Display.printf("Accel Y: %.2f", ay); y += 12;
  M5.Display.setCursor(5, y);
  M5.Display.printf("Accel Z: %.2f", az); y += 18;

  // Gyroscope
  M5.Display.setCursor(5, y);
  M5.Display.printf("Gyro X: %.2f", gx); y += 12;
  M5.Display.setCursor(5, y);
  M5.Display.printf("Gyro Y: %.2f", gy); y += 12;
  M5.Display.setCursor(5, y);
  M5.Display.printf("Gyro Z: %.2f", gz); y += 18;

  // Orientation
  M5.Display.setCursor(5, y);
  M5.Display.printf("Pitch: %.2f", pitch); y += 12;
  M5.Display.setCursor(5, y);
  M5.Display.printf("Roll:  %.2f", roll);  y += 12;
  M5.Display.setCursor(5, y);
  M5.Display.printf("Yaw:   %.2f", yaw);

  delay(50); // readable refresh rate
}

// ================================================================
//  Utility: Clear screen and reset text formatting
// ================================================================
void resetScreen() {
  M5.Display.fillScreen(COL_BG);
  M5.Display.setTextColor(COL_TEXT, COL_BG);
  M5.Display.setTextSize(2);
}

// End of file
