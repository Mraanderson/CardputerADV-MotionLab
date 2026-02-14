#ifndef SCK
#define SCK 5
#define MISO 0
#define MOSI 4
#define SS 1
#endif

#include <Arduino.h>
#include <M5Unified.h>
#include <cmath>
#include <M5Cardputer.h>
#include <Preferences.h>

struct P2 { int x, y; };
struct P3 { float x, y, z; };

M5Canvas canvas(&M5.Display); 
Preferences prefs;

enum AppMode { MODE_SPLASH, MODE_MENU, MODE_CUBE, MODE_LEVEL, MODE_LAUNCH, MODE_GAME, MODE_GRAPH, MODE_RAW };
AppMode currentMode = MODE_SPLASH;

float ax, ay, az, gx, gy, gz;
float zoomScale = 90.0f;
float highG = 0;

P3 cubeVerts[8] = {{-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},{-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}};
int cubeEdges[12][2] = {{0,1},{1,2},{2,3},{3,0},{4,5},{5,6},{6,7},{7,4},{0,4},{1,5},{2,6},{3,7}};

bool exitRequested() {
    return M5Cardputer.Keyboard.isKeyPressed(0x4C) || M5Cardputer.Keyboard.isKeyPressed(0x2A);
}

// Forward Declarations
void drawSplash(); void drawMenu(); void demoCube(); void demoLevel(); 
void demoLaunch(); void demoTiltGame(); void demoGraph(); void demoRaw();

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5Cardputer.begin();
    M5.Display.setRotation(1);
    canvas.createSprite(M5.Display.width(), M5.Display.height());
    
    prefs.begin("motion-lab", false);
    highG = prefs.getFloat("highG", 0.0f);

    if (!M5.Imu.begin()) { while (true) delay(100); }
}

void loop() {
    M5.update();
    M5Cardputer.update();
    switch (currentMode) {
        case MODE_SPLASH: drawSplash();   break;
        case MODE_MENU:   drawMenu();     break;
        case MODE_CUBE:   demoCube();     break;
        case MODE_LEVEL:  demoLevel();    break;
        case MODE_LAUNCH: demoLaunch();   break;
        case MODE_GAME:   demoTiltGame(); break;
        case MODE_GRAPH:  demoGraph();    break;
        case MODE_RAW:    demoRaw();      break;
    }
}

void drawSplash() {
    static uint32_t startTime = millis();
    canvas.fillScreen(BLACK);
    canvas.setTextColor(GREEN);
    canvas.setTextSize(2);
    canvas.drawCenterString("Cardputer ADV", 120, 30);
    canvas.drawCenterString("Motion Lab", 120, 60);
    canvas.setTextSize(1);
    canvas.drawCenterString("v0.5 - shake it up", 120, 100);
    canvas.pushSprite(0,0);
    if (millis() - startTime > 2000) { currentMode = MODE_MENU; }
}

void drawMenu() {
    canvas.fillScreen(BLACK);
    canvas.setTextColor(GREEN);
    canvas.setTextSize(2);
    canvas.setCursor(10, 10); canvas.print("IMU Demo Menu");
    canvas.setTextSize(1);
    int y = 40;
    const char* opts[] = {"1. 3D Cube", "2. Bubble Level", "3. Tilt Game", "4. G-Force Mode", "5. IMU Graph", "6. Raw Viewer"};
    for(int i=0; i<6; i++) { canvas.setCursor(10, y); canvas.print(opts[i]); y+=15; }
    canvas.pushSprite(0,0);

    if (M5Cardputer.Keyboard.isKeyPressed('1')) currentMode = MODE_CUBE;
    if (M5Cardputer.Keyboard.isKeyPressed('2')) currentMode = MODE_LEVEL;
    if (M5Cardputer.Keyboard.isKeyPressed('3')) currentMode = MODE_GAME;
    if (M5Cardputer.Keyboard.isKeyPressed('4')) currentMode = MODE_LAUNCH;
    if (M5Cardputer.Keyboard.isKeyPressed('5')) currentMode = MODE_GRAPH;
    if (M5Cardputer.Keyboard.isKeyPressed('6')) currentMode = MODE_RAW;
}

// --- G-FORCE MODE ---
void demoLaunch() {
    if (exitRequested()) { currentMode = MODE_MENU; return; }
    static float currentMax = 0;
    static uint32_t peakTime = 0;

    M5.Imu.getAccel(&ax, &ay, &az);
    float force = sqrt(ax*ax + ay*ay + az*az);

    // Record high peaks
    if (force > 1.5f && force > currentMax) {
        currentMax = force;
        peakTime = millis();
        if (currentMax > highG) {
            highG = currentMax;
            prefs.putFloat("highG", highG);
        }
    }

    // Reset local record
    if (M5Cardputer.Keyboard.isKeyPressed('r')) {
        highG = 0; currentMax = 0;
        prefs.putFloat("highG", 0);
    }

    canvas.fillScreen(BLACK);
    canvas.setTextSize(1);
    canvas.setTextColor(YELLOW);
    canvas.setCursor(5, 5);
    canvas.printf("ALL-TIME RECORD: %.2fG", highG);
    canvas.drawFastHLine(0, 18, 240, DARKGREY); 

    if (currentMax < 1.5f || millis() - peakTime > 3000) {
        currentMax = 0; 
        canvas.setTextColor(WHITE); canvas.setTextSize(2);
        canvas.drawCenterString("Ready for G-force", 120, 55);
    } else {
        canvas.setTextColor(currentMax >= highG ? GREEN : RED);
        canvas.setTextSize(2);
        canvas.drawCenterString("Peak Force", 120, 35);
        canvas.setTextSize(5); canvas.setCursor(30, 65);
        canvas.printf("%.2fG", currentMax);
        int bar = map(millis() - peakTime, 0, 3000, 240, 0);
        canvas.fillRect(0, 130, bar, 5, GREEN);
    }

    // Live Footer
    canvas.setTextSize(1);
    canvas.setTextColor(DARKGREY);
    canvas.drawFastHLine(0, 115, 240, canvas.color565(40, 40, 40));
    canvas.setCursor(5, 122);
    canvas.printf("Live:%.2fG (1.00 = Gravity)", force);
    canvas.drawRightString("R = Reset", 235, 122);
    canvas.pushSprite(0, 0);
}

// --- REST OF THE TOOLS ---
void demoCube() {
    if (exitRequested()) { currentMode = MODE_MENU; return; }
    
    // Zoom Logic with 0-200 Constraints
    if (M5Cardputer.Keyboard.isKeyPressed('+') || M5Cardputer.Keyboard.isKeyPressed('=')) {
        zoomScale += 2.0f;
        if (zoomScale > 200.0f) zoomScale = 200.0f; // Upper Limit
    }
    if (M5Cardputer.Keyboard.isKeyPressed('-') || M5Cardputer.Keyboard.isKeyPressed('_')) {
        zoomScale -= 2.0f;
        if (zoomScale < 0.0f) zoomScale = 0.0f;     // Lower Limit
    }
    
    // Reset to 100
    if (M5Cardputer.Keyboard.isKeyPressed('0')) zoomScale = 100.0f;

    M5.Imu.getAccel(&ax, &ay, &az);
    float p = atan2(ay, az), r = atan2(-ax, sqrtf(ay*ay + az*az));
    
    canvas.fillScreen(BLACK);
    
    // 3D Projection Math
    P2 pts[8];
    for (int i=0; i<8; i++) {
        float cp=cos(p), sp=sin(p), cr=cos(r), sr=sin(r);
        float x1 = cubeVerts[i].x, y1 = cubeVerts[i].y*cp - cubeVerts[i].z*sp, z1v = cubeVerts[i].y*sp + cubeVerts[i].z*cp;
        float x2 = x1*cr - z1v*sr, z2v = x1*sr + z1v*cr;
        
        // Perspective Projection (using a fixed depth of 4.0)
        float inv = 1.0f / (z2v + 4.0f);
        pts[i] = P2{(int)(x2*zoomScale*inv + 120), (int)(y1*zoomScale*inv + 67)};
    }
    
    // Draw Wireframe
    for (int i=0; i<12; i++) {
        canvas.drawLine(pts[cubeEdges[i][0]].x, pts[cubeEdges[i][0]].y, 
                        pts[cubeEdges[i][1]].x, pts[cubeEdges[i][1]].y, GREEN);
    }

    // Footer
    canvas.setTextSize(1);
    canvas.setTextColor(DARKGREY);
    canvas.drawFastHLine(0, 115, 240, canvas.color565(40, 40, 40));
    canvas.setCursor(5, 122);
    canvas.print("Zoom:+/- | Reset:0");
    canvas.drawRightString("Scale: " + String((int)zoomScale), 235, 122);

    canvas.pushSprite(0, 0);
}

void demoLevel() {
    if (exitRequested()) { currentMode = MODE_MENU; return; }
    M5.Imu.getAccel(&ax, &ay, &az);
    canvas.fillScreen(BLACK);
    uint16_t gridCol = canvas.color565(0, 100, 200);
    canvas.drawLine(120, 0, 120, 135, gridCol);
    canvas.drawLine(0, 67, 240, 67, gridCol);
    canvas.drawCircle(120, 67, 60, gridCol);
    canvas.fillCircle(120 + (ax*60), 67 + (-ay*60), 12, YELLOW);
    canvas.pushSprite(0,0);
}

void demoTiltGame() {
    static bool firstRun = true, isBubbleMode = true, goalInit = false;
    static float px, py; static int gx_g, gy_g;
    if (exitRequested()) { currentMode = MODE_MENU; firstRun = true; goalInit = false; return; }
    if (firstRun) { px = 120; py = 67; firstRun = false; }
    if (!goalInit) { gx_g = 20+(rand()%200); gy_g = 20+(rand()%100); goalInit = true; }
    M5.Imu.getAccel(&ax, &ay, &az);
    float s = 4.0f;
    if (isBubbleMode) { px += ax*s; py += -ay*s; } else { px -= ax*s; py -= -ay*s; }
    px = constrain(px, 10, 230); py = constrain(py, 10, 125);
    if (isBubbleMode) {
        canvas.fillScreen(canvas.color565(180, 220, 255));
        canvas.setTextColor(canvas.color565(0, 50, 150));
        canvas.drawCenterString("BUBBLE UNDER ICE", 120, 5);
        canvas.fillCircle(gx_g, gy_g, 10, WHITE); canvas.fillCircle(gx_g, gy_g, 6, canvas.color565(0,0,100));
        canvas.fillCircle(px, py, 8, WHITE);
    } else {
        canvas.fillScreen(WHITE); canvas.setTextColor(DARKGREY);
        canvas.drawCenterString("BALL ON TABLE", 120, 5);
        canvas.fillCircle(gx_g, gy_g, 8, BLACK); canvas.fillCircle(px, py, 8, RED);
    }
    canvas.pushSprite(0, 0);
    if (pow(px-gx_g, 2) + pow(py-gy_g, 2) < 144) { isBubbleMode = !isBubbleMode; goalInit = false; M5.Display.fillScreen(YELLOW); delay(150); }
}

void demoGraph() {
    if (exitRequested()) { currentMode = MODE_MENU; return; }
    static float bX[240], bY[240], bZ[240]; static int idx = 0;
    M5.Imu.getAccel(&ax, &ay, &az);
    bX[idx] = ax * 40.0f; bY[idx] = ay * 40.0f; bZ[idx] = az * 40.0f;
    idx = (idx + 1) % 240;
    canvas.fillScreen(BLACK);
    canvas.drawLine(0, 67, 240, 67, canvas.color565(40, 40, 40));
    for(int i = 0; i < 239; i++) {
        int i1 = (idx + i) % 240, i2 = (idx + i + 1) % 240;
        canvas.drawLine(i, 67 - bX[i1], i + 1, 67 - bX[i2], RED);
        canvas.drawLine(i, 67 - bY[i1], i + 1, 67 - bY[i2], GREEN);
        canvas.drawLine(i, 67 - bZ[i1], i + 1, 67 - bZ[i2], BLUE);
    }
    canvas.pushSprite(0, 0);
}
void demoRaw() {
    if (exitRequested()) { currentMode = MODE_MENU; return; }
    M5.Imu.getAccel(&ax, &ay, &az); M5.Imu.getGyro(&gx, &gy, &gz);
    float p = atan2(ay, az) * 57.3f, r = atan2(-ax, sqrtf(ay * ay + az * az)) * 57.3f;
    
    canvas.fillScreen(BLACK); 
    canvas.setTextColor(GREEN); canvas.setTextSize(2);
    canvas.setCursor(5, 5); canvas.println("IMU SENSOR DATA");
    canvas.drawFastHLine(0, 25, 240, canvas.color565(0, 100, 0));
    
    canvas.setTextSize(1);
    
    // --- Column 1: ACCEL ---
    int startY = 35;
    canvas.setTextColor(RED);
    canvas.setCursor(5, startY);      canvas.print("ACCEL [G]");
    canvas.setCursor(5, startY + 10); canvas.printf("X: %+6.2f", ax);
    canvas.setCursor(5, startY + 20); canvas.printf("Y: %+6.2f", ay);
    canvas.setCursor(5, startY + 30); canvas.printf("Z: %+6.2f", az);
    
    // --- Column 2: GYRO ---
    canvas.setTextColor(BLUE);
    canvas.setCursor(120, startY);      canvas.print("GYRO [deg/s]");
    canvas.setCursor(120, startY + 10); canvas.printf("X: %+7.1f", gx);
    canvas.setCursor(120, startY + 20); canvas.printf("Y: %+7.1f", gy);
    canvas.setCursor(120, startY + 30); canvas.printf("Z: %+7.1f", gz);
    
    // --- Bottom: PITCH/ROLL ---
    canvas.setTextColor(YELLOW); 
    canvas.setCursor(5, 115); 
    canvas.printf("PITCH: %6.1f deg  ROLL: %6.1f deg", p, r);
    
    canvas.pushSprite(0, 0);
    // Removed the long delay(100) to keep the UI snappy
}
