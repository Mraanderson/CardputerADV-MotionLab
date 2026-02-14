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
float zoomScale = 90.0f;   // initial cube size

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
//  Utility: unified exit key handling (Delete only)
// ================================================================
bool exitRequested() {
    return M5Cardputer.Keyboard.isKeyPressed(0x4C) ||  // Delete
           M5Cardputer.Keyboard.isKeyPressed(0x2A);    // Backspace (often Delete on ADV)
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
//  Splash Screen
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
    tw = M5.Display.textWidth("Tilt, rotate & shake to explore the IMU!");
    M5.Display.setCursor((M5.Display.width() - tw) / 2, 100);
    M5.Display.print(" Tilt/rotate/shake and explore the IMU!\n");
    M5.Display.print("\n v0.04");
  }

  // Auto‑advance only (no buttons)
  if (millis() - startTime > 3000) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
  }
}

// ================================================================
//  Menu Screen (keyboard only)
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

  // Keyboard‑only menu navigation
  if (M5Cardputer.Keyboard.isKeyPressed('1')) currentMode = MODE_CUBE;
  if (M5Cardputer.Keyboard.isKeyPressed('2')) currentMode = MODE_LEVEL;
  if (M5Cardputer.Keyboard.isKeyPressed('3')) currentMode = MODE_GAME;
  if (M5Cardputer.Keyboard.isKeyPressed('4')) currentMode = MODE_GRAPH;
  if (M5Cardputer.Keyboard.isKeyPressed('5')) currentMode = MODE_RAW;

  if (currentMode != MODE_MENU) first = true;
}

// ================================================================
//  3D Cube Demo (IMU‑Driven, Live Zoom)
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

// Projection using live zoom
P2 projectPoint(P3 p) {
  float scale   = zoomScale;
  float zOffset = 4.0f;
  float inv     = 1.0f / (p.z + zOffset);

  return {
    (int)(p.x * scale * inv + M5.Display.width()  / 2),
    (int)(p.y * scale * inv + M5.Display.height() / 2)
  };
}

// Rotation
P3 rotatePoint(P3 v, float pitch, float roll, float yaw) {
  float cp = cos(pitch), sp = sin(pitch);
  float cr = cos(roll),  sr = sin(roll);
  float cy = cos(yaw),   sy = sin(yaw);

  float x1 = v.x * cy - v.z * sy;
  float z1 = v.x * sy + v.z * cy;

  float y2 = v.y * cp - z1 * sp;
  float z2 = v.y * sp + z1 * cp;

  float x3 = x1 * cr - y2 * sr;
  float y3 = x1 * sr + y2 * cr;

  return {x3, y3, z2};
}

void demoCube() {
  M5.update();

  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  // Zoom controls
  if (M5Cardputer.Keyboard.isKeyPressed('+') ||
      M5Cardputer.Keyboard.isKeyPressed('=')) {
    zoomScale += 2.0f;
    if (zoomScale > 140.0f) zoomScale = 140.0f;
  }

  if (M5Cardputer.Keyboard.isKeyPressed('-') ||
      M5Cardputer.Keyboard.isKeyPressed('_')) {
    zoomScale -= 2.0f;
    if (zoomScale < 20.0f) zoomScale = 20.0f;
  }

  if (M5Cardputer.Keyboard.isKeyPressed('0')) {
    zoomScale = 90.0f;
  }

  // Read IMU
  M5.Imu.getAccel(&ax, &ay, &az);

  pitch = atan2(ay, az) * 57.2958f;
  roll  = atan2(-ax, sqrtf(ay*ay + az*az)) * 57.2958f;
  yaw   = 0;

  float p = pitch * 0.0174533f;
  float r = roll  * 0.0174533f;
  float y = yaw   * 0.0174533f;

  M5.Display.fillScreen(BLACK);

  P2 pts[8];
  P3 rpVerts[8];

  for (int i = 0; i < 8; i++) {
    rpVerts[i] = rotatePoint(cubeVerts[i], p, r, y);
    pts[i]     = projectPoint(rpVerts[i]);
  }

  for (int i = 0; i < 12; i++) {
    int a = cubeEdges[i][0];
    int b = cubeEdges[i][1];

    float shade = (rpVerts[a].z + rpVerts[b].z + 2.0f) * 0.25f;
    shade = constrain(shade, 0.2f, 1.0f);

    uint8_t intensity = (uint8_t)(shade * 255);
    uint16_t col = M5.Display.color565(0, intensity, intensity);

    M5.Display.drawLine(pts[a].x, pts[a].y, pts[b].x, pts[b].y, col);
  }

  delay(16);
}

// ================================================================
//  Bubble Level Demo
// ================================================================
void demoLevel() {
  M5.update();

  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  M5.Imu.getAccel(&ax, &ay, &az);

  int cx = M5.Display.width()  / 2;
  int cy = M5.Display.height() / 2;

  float scale = 80.0f;

  int bx = cx + (int)(ax * scale);
  int by = cy + (int)(-ay * scale);

  bx = constrain(bx, 10, M5.Display.width()  - 10);
  by = constrain(by, 10, M5.Display.height() - 10);

  M5.Display.fillScreen(BLACK);

  uint16_t gridCol = M5.Display.color565(0, 180, 255);
  M5.Display.drawLine(cx, 0, cx, M5.Display.height(), gridCol);
  M5.Display.drawLine(0, cy, M5.Display.width(), cy, gridCol);

  int radius = min(cx, cy) - 5;
  M5.Display.drawCircle(cx, cy, radius, gridCol);

  uint16_t bubbleCol = M5.Display.color565(255, 255, 0);
  M5.Display.fillCircle(bx, by, 12, bubbleCol);

  delay(16);
}

// ================================================================
//  Tilt Game Demo
// ================================================================
void demoTiltGame() {
  M5.update();

  static bool firstRun = true;
  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    firstRun = true;
    return;
  }

  M5.Imu.getAccel(&ax, &ay, &az);

  int w = M5.Display.width();
  int h = M5.Display.height();

  static float px;
  static float py;

  if (firstRun) {
    px = w / 2.0f;
    py = h / 2.0f;
    firstRun = false;
  }

  float speed = 4.0f;

  px += ax * speed;
  py += -ay * speed;

  if (px < 10) px = 10;
  if (px > w - 10) px = w - 10;
  if (py < 10) py = 10;
  if (py > h - 10) py = h - 10;

  M5.Display.fillScreen(BLACK);

  uint16_t borderCol = M5.Display.color565(0, 200, 255);
  M5.Display.drawRect(5, 5, w - 10, h - 10, borderCol);

  uint16_t playerCol = M5.Display.color565(255, 0, 200);
  M5.Display.fillCircle((int)px, (int)py, 8, playerCol);

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

  int dx = (int)px - gx;
  int dy = (int)py - gy;

  if (dx * dx + dy * dy < 14 * 14) {
    gx = 20 + (rand() % (w - 40));
    gy = 20 + (rand() % (h - 40));

    M5.Display.fillScreen(M5.Display.color565(255, 255, 0));
    delay(80);
  }

  delay(16);
}

// ================================================================
//  IMU Graph Demo
// ================================================================
static const int GRAPH_W = 240;
static const int GRAPH_H = 135;

static int graphIndex = 0;

static float bufAx[GRAPH_W];
static float bufAy[GRAPH_W];
static float bufAz[GRAPH_W];

static float bufGx[GRAPH_W];
static float bufGy[GRAPH_W];
static float bufGz[GRAPH_W];

void demoGraph() {
  M5.update();

  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  M5.Imu.getAccel(&ax, &ay, &az);
  M5.Imu.getGyro(&gx, &gy, &gz);

  bufAx[graphIndex] = ax;
  bufAy[graphIndex] = ay;
  bufAz[graphIndex] = az;

  bufGx[graphIndex] = gx * 0.05f;
  bufGy[graphIndex] = gy * 0.05f;
  bufGz[graphIndex] = gz * 0.05f;

  graphIndex = (graphIndex + 1) % GRAPH_W;

  M5.Display.fillScreen(BLACK);

  uint16_t midCol = M5.Display.color565(80, 80, 80);
  M5.Display.drawLine(0, GRAPH_H / 2, GRAPH_W, GRAPH_H / 2, midCol);

  for (int i = 0; i < GRAPH_W - 1; i++) {
    int idx1 = (graphIndex + i)     % GRAPH_W;
    int idx2 = (graphIndex + i + 1) % GRAPH_W;

    auto mapY = [&](float v) {
      return (int)(GRAPH_H / 2 - v * 20.0f);
    };

    uint16_t colAx = M5.Display.color565(255, 0,   0);
    uint16_t colAy = M5.Display.color565(0,   255, 0);
    uint16_t colAz = M5.Display.color565(0,   128, 255);

    uint16_t colGx = M5.Display.color565(255, 255, 0);
    uint16_t colGy = M5.Display.color565(255, 0,   255);
    uint16_t colGz = M5.Display.color565(0,   255, 255);

    M5.Display.drawLine(i, mapY(bufAx[idx1]), i + 1, mapY(bufAx[idx2]), colAx);
    M5.Display.drawLine(i, mapY(bufAy[idx1]), i + 1, mapY(bufAy[idx2]), colAy);
    M5.Display.drawLine(i, mapY(bufAz[idx1]), i + 1, mapY(bufAz[idx2]), colAz);

    M5.Display.drawLine(i, mapY(bufGx[idx1]), i + 1, mapY(bufGx[idx2]), colGx);
    M5.Display.drawLine(i, mapY(bufGy[idx1]), i + 1, mapY(bufGy[idx2]), colGy);
    M5.Display.drawLine(i, mapY(bufGz[idx1]), i + 1, mapY(bufGz[idx2]), colGz);
  }

  delay(16);
}

// ================================================================
//  Raw Data Viewer
// ================================================================
void demoRaw() {
  M5.update();

  if (exitRequested()) {
    currentMode = MODE_MENU;
    M5.Display.fillScreen(COL_BG);
    return;
  }

  M5.Imu.getAccel(&ax, &ay, &az);
  M5.Imu.getGyro(&gx, &gy, &gz);

  pitch = atan2(ay, az) * 57.2958f;
  roll  = atan2(-ax, sqrtf(ay*ay + az*az)) * 57.2958f;
  yaw   = 0;

  // Clear screen
  M5.Display.fillScreen(COL_BG);
  M5.Display.setTextColor(COL_TEXT, COL_BG);
  M5.Display.setTextSize(1);  // compact for ADV screen

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
