#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <time.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
const char* ssid = "Ggg";
const char* password = "00000000";

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST = UTC+5:30 = 19800 sec

// Pixel burn prevention offset
int offsetX = 0;
int offsetY = 0;
int offsetDir = 1;
unsigned long lastOffsetChange = 0;

// Screen mode
enum Mode { WATCH, EYES };
Mode currentMode = WATCH;
unsigned long lastModeSwitch = 0;
#define MODE_DURATION 6000  // Switch every 6 seconds

// Eye animation
enum Emotion { NORMAL, BLINK, HAPPY, SAD, ANGRY, SURPRISED, SLEEPY };
Emotion currentEmotion = NORMAL;
unsigned long lastEmotionChange = 0;
unsigned long lastBlink = 0;
bool isBlinking = false;
int blinkFrame = 0;

// Eye positions
int leftEyeX = 35;
int leftEyeY = 32;
int rightEyeX = 93;
int rightEyeY = 32;
int eyeRadius = 14;

// Eye movement
float eyeMoveX = 0;
float eyeMoveY = 0;
float eyeTargetX = 0;
float eyeTargetY = 0;

// Watch animation offset (slight float)
float watchOffsetX = 0;
float watchOffsetY = 0;
float watchAngle = 0;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED not found!");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(20, 25);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(500);
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    timeClient.begin();
    timeClient.update();
    display.clearDisplay();
    display.setCursor(25, 25);
    display.println("WiFi Connected!");
    display.display();
    delay(1000);
  } else {
    display.clearDisplay();
    display.setCursor(15, 20);
    display.println("WiFi Failed!");
    display.setCursor(10, 35);
    display.println("Using local time");
    display.display();
    delay(1500);
  }
}

// ─────────────────────────────────────────────
// WATCH FUNCTIONS
// ─────────────────────────────────────────────

void drawWatch() {
  timeClient.update();

  int h = timeClient.getHours();
  int m = timeClient.getMinutes();
  int s = timeClient.getSeconds();

  // Slight floating animation to prevent burn-in
  watchAngle += 0.05;
  watchOffsetX = sin(watchAngle) * 2;
  watchOffsetY = cos(watchAngle * 0.7) * 1.5;

  int ox = (int)watchOffsetX;
  int oy = (int)watchOffsetY;

  display.clearDisplay();

  // Draw outer border
  display.drawRoundRect(2 + ox, 2 + oy, 124, 60, 6, SSD1306_WHITE);

  // Draw time HH:MM:SS
  char timeBuf[9];
  sprintf(timeBuf, "%02d:%02d:%02d", h, m, s);
  display.setTextSize(2);
  int tw = strlen(timeBuf) * 12;
  display.setCursor((128 - tw) / 2 + ox, 8 + oy);
  display.println(timeBuf);

  // Divider line
  display.drawLine(10 + ox, 34 + oy, 118 + ox, 34 + oy, SSD1306_WHITE);

  // Day of week
  String days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
  int dayNum = timeClient.getDay();
  String dayStr = days[dayNum];

  // Date (we estimate using epoch)
  unsigned long epoch = timeClient.getEpochTime();
  int totalDays = epoch / 86400;
  int y = 1970, mo = 1, d = 1;
  // Simple date calculation
  while (true) {
    int daysInYear = ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) ? 366 : 365;
    if (totalDays >= daysInYear) { totalDays -= daysInYear; y++; }
    else break;
  }
  int monthDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  if ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0) monthDays[1] = 29;
  for (mo = 0; mo < 12; mo++) {
    if (totalDays >= monthDays[mo]) totalDays -= monthDays[mo];
    else break;
  }
  d = totalDays + 1;
  mo++;

  char dateBuf[20];
  String months[] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
  sprintf(dateBuf, "%s %02d %s %d", dayStr.c_str(), d, months[mo-1].c_str(), y);

  display.setTextSize(1);
  int dw = strlen(dateBuf) * 6;
  display.setCursor((128 - dw) / 2 + ox, 40 + oy);
  display.println(dateBuf);

  // WiFi indicator
  if (WiFi.status() == WL_CONNECTED) {
    display.setCursor(105 + ox, 50 + oy);
    display.setTextSize(1);
    display.print("WiFi");
  }

  display.display();
}

// ─────────────────────────────────────────────
// EYE FUNCTIONS
// ─────────────────────────────────────────────

void drawEye(int cx, int cy, int r, Emotion emo, bool blink, int blinkF) {
  if (blink || emo == BLINK) {
    int h = map(blinkF, 0, 5, r, 0);
    display.fillRoundRect(cx - r, cy - h/2, r*2, h, h/3, SSD1306_WHITE);
    return;
  }

  switch (emo) {
    case NORMAL:
      display.fillCircle(cx, cy, r, SSD1306_WHITE);
      display.fillCircle(cx + 4, cy - 3, r/3, SSD1306_BLACK); // pupil
      break;

    case HAPPY:
      // Half circle (smile eyes)
      for (int i = 0; i < r; i++) {
        display.drawFastHLine(cx - r + i/2, cy + i - r, r*2 - i, SSD1306_WHITE);
      }
      display.fillRect(cx - r, cy - r, r*2, r, SSD1306_BLACK); // mask top
      display.drawCircle(cx, cy, r, SSD1306_WHITE);
      display.fillRect(cx - r - 1, cy - r - 1, r*2 + 2, r + 1, SSD1306_BLACK);
      display.fillCircle(cx, cy, r, SSD1306_WHITE);
      display.fillRect(cx - r - 1, cy, r*2 + 2, r + 3, SSD1306_BLACK);
      break;

    case SAD:
      display.fillCircle(cx, cy, r, SSD1306_WHITE);
      display.fillCircle(cx + 4, cy - 3, r/3, SSD1306_BLACK);
      // sad eyebrow
      display.drawLine(cx - r, cy - r - 3, cx + r, cy - r + 3, SSD1306_WHITE);
      break;

    case ANGRY:
      display.fillCircle(cx, cy, r, SSD1306_WHITE);
      display.fillCircle(cx + 3, cy, r/3, SSD1306_BLACK);
      // angry eyebrow
      if (cx < 64)
        display.drawLine(cx - r, cy - r - 2, cx + r/2, cy - r + 4, SSD1306_WHITE);
      else
        display.drawLine(cx - r/2, cy - r + 4, cx + r, cy - r - 2, SSD1306_WHITE);
      break;

    case SURPRISED:
      display.drawCircle(cx, cy, r + 2, SSD1306_WHITE);
      display.drawCircle(cx, cy, r, SSD1306_WHITE);
      display.fillCircle(cx, cy, r/2, SSD1306_WHITE);
      display.fillCircle(cx + 2, cy - 2, r/4, SSD1306_BLACK);
      break;

    case SLEEPY:
      display.fillCircle(cx, cy, r, SSD1306_WHITE);
      display.fillRect(cx - r - 1, cy - r - 1, r*2 + 2, r/2 + 2, SSD1306_BLACK);
      display.fillCircle(cx + 3, cy + 2, r/3, SSD1306_BLACK);
      break;

    default:
      display.fillCircle(cx, cy, r, SSD1306_WHITE);
      break;
  }
}

void drawEyes() {
  unsigned long now = millis();

  // Change emotion randomly
  if (now - lastEmotionChange > 4000) {
    int r = random(0, 6);
    Emotion emotions[] = {NORMAL, HAPPY, SAD, ANGRY, SURPRISED, SLEEPY};
    currentEmotion = emotions[r];
    lastEmotionChange = now;

    // New eye target position (slight look around)
    eyeTargetX = random(-4, 5);
    eyeTargetY = random(-3, 4);
  }

  // Smooth eye movement
  eyeMoveX += (eyeTargetX - eyeMoveX) * 0.1;
  eyeMoveY += (eyeTargetY - eyeMoveY) * 0.1;

  // Random blink
  if (!isBlinking && now - lastBlink > random(2000, 5000)) {
    isBlinking = true;
    blinkFrame = 0;
    lastBlink = now;
  }

  if (isBlinking) {
    blinkFrame++;
    if (blinkFrame >= 6) {
      isBlinking = false;
      blinkFrame = 0;
    }
  }

  // Pixel burn offset (slight drift)
  if (now - lastOffsetChange > 5000) {
    offsetX = random(-2, 3);
    offsetY = random(-1, 2);
    lastOffsetChange = now;
  }

  int lx = leftEyeX + (int)eyeMoveX + offsetX;
  int ly = leftEyeY + (int)eyeMoveY + offsetY;
  int rx = rightEyeX + (int)eyeMoveX + offsetX;
  int ry = rightEyeY + (int)eyeMoveY + offsetY;

  display.clearDisplay();

  // Draw face border (subtle)
  display.drawRoundRect(0, 0, 128, 64, 8, SSD1306_WHITE);

  drawEye(lx, ly, eyeRadius, currentEmotion, isBlinking, blinkFrame);
  drawEye(rx, ry, eyeRadius, currentEmotion, isBlinking, blinkFrame);

  // Show emotion label
  display.setTextSize(1);
  String emoLabels[] = {"Normal","Blink","Happy","Sad","Angry","Surprised","Sleepy"};
  String label = emoLabels[(int)currentEmotion];
  int lw = label.length() * 6;
  display.setCursor((128 - lw) / 2, 54);
  display.println(label);

  display.display();
}

// ─────────────────────────────────────────────
// MAIN LOOP
// ─────────────────────────────────────────────

void loop() {
  unsigned long now = millis();

  // Switch between Watch and Eyes every 6 seconds
  if (now - lastModeSwitch > MODE_DURATION) {
    currentMode = (currentMode == WATCH) ? EYES : WATCH;
    lastModeSwitch = now;
    display.clearDisplay();
    display.display();
    delay(200); // Small blank gap between transitions
  }

  if (currentMode == WATCH) {
    drawWatch();
    delay(100);
  } else {
    drawEyes();
    delay(50);
  }
}