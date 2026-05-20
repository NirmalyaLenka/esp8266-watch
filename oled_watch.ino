/*
  OLED Watch Face for ESP-12F (ESP8266)
  Display : 0.96" SSD1306 OLED (128x64) via I2C
  WiFi    : Loda lebo (open / no password)
  Time    : NTP synced (UTC+2 — adjust TZ_OFFSET below)

  Wiring:
    OLED VCC  → 3.3V
    OLED GND  → GND
    OLED SDA  → GPIO4  (D2)
    OLED SCL  → GPIO5  (D1)

  Libraries needed (install via Arduino Library Manager):
    - Adafruit SSD1306
    - Adafruit GFX Library
    - ESP8266WiFi       (built-in with ESP8266 board package)
    - NTPClient         (by Fabrice Weinberg)
    - WiFiUdp           (built-in)
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// ── Display ──────────────────────────────────────────────
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1   // no reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── WiFi ─────────────────────────────────────────────────
const char* WIFI_SSID     = "Loda lebo";
const char* WIFI_PASSWORD = "";           // open network

// ── NTP ──────────────────────────────────────────────────
// Adjust TZ_OFFSET_SEC to your timezone:
//   UTC+2  → 7200   (South Africa / SAST)
//   UTC+5:30→ 19800 (India / IST)
//   UTC+0  → 0
#define TZ_OFFSET_SEC 7200

WiFiUDP   ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", TZ_OFFSET_SEC, 60000);

// ── Step counter (placeholder — wire a pedometer or use 0) ─
int stepCount = 0;
const int STEP_GOAL = 10000;

// ── Helpers ───────────────────────────────────────────────
String zeroPad(int n) {
  return (n < 10) ? "0" + String(n) : String(n);
}

String getDayName(int wday) {
  const char* days[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
  return String(days[wday % 7]);
}

String getMonthName(int month) {
  const char* months[] = {
    "JAN","FEB","MAR","APR","MAY","JUN",
    "JUL","AUG","SEP","OCT","NOV","DEC"
  };
  return String(months[(month - 1) % 12]);
}

// ── Draw the watch face ───────────────────────────────────
void drawWatchFace(int h24, int min, int sec,
                   int day, int month, int year, int wday) {

  display.clearDisplay();

  // ── Convert to 12h ──
  bool isPM  = (h24 >= 12);
  int  h12   = h24 % 12;
  if (h12 == 0) h12 = 12;

  String timeStr = zeroPad(h12) + ":" + zeroPad(min);
  String ampm    = isPM ? "PM" : "AM";

  // ── Day name (top centre) ──
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  String dayLabel = getDayName(wday);
  int dw = dayLabel.length() * 6;
  display.setCursor((SCREEN_WIDTH - dw) / 2, 0);
  display.print(dayLabel);

  // ── Divider ──
  display.drawLine(10, 10, 117, 10, SSD1306_WHITE);

  // ── AM/PM badge — top-left of time block ──
  display.setTextSize(1);
  display.setCursor(2, 14);
  display.print(ampm);

  // ── Big time (centred, size 3 = 18px tall chars) ──
  display.setTextSize(3);
  int tw = timeStr.length() * 18;
  display.setCursor((SCREEN_WIDTH - tw) / 2, 16);
  display.print(timeStr);

  // ── Divider ──
  display.drawLine(10, 40, 117, 40, SSD1306_WHITE);

  // ── Date ──
  display.setTextSize(1);
  String dateStr = zeroPad(day) + " " + getMonthName(month) + " " + String(year);
  int datew = dateStr.length() * 6;
  display.setCursor((SCREEN_WIDTH - datew) / 2, 43);
  display.print(dateStr);

  // ── Step progress bar ──
  int barX = 10, barY = 56, barW = 88, barH = 5;
  display.drawRect(barX, barY, barW, barH, SSD1306_WHITE);
  int filled = map(min(stepCount, STEP_GOAL), 0, STEP_GOAL, 0, barW - 2);
  if (filled > 0)
    display.fillRect(barX + 1, barY + 1, filled, barH - 2, SSD1306_WHITE);

  // ── Step count label ──
  display.setTextSize(1);
  display.setCursor(101, 56);
  // Show as "6.2k" style if large
  if (stepCount >= 1000)
    display.print(String(stepCount / 1000) + "." + String((stepCount % 1000) / 100) + "k");
  else
    display.print(String(stepCount));

  display.display();
}

// ── Reconnect helper ─────────────────────────────────────
void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    tries++;
  }
}

// ── Setup ─────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 not found — check wiring");
    while (true);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Splash
  display.setTextSize(1);
  display.setCursor(20, 20);
  display.print("Connecting WiFi...");
  display.display();

  // Connect WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(500);
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    display.clearDisplay();
    display.setCursor(28, 20);
    display.print("WiFi OK!");
    display.setCursor(20, 32);
    display.print("Syncing time...");
    display.display();
    delay(800);
  } else {
    display.clearDisplay();
    display.setCursor(10, 20);
    display.print("WiFi failed!");
    display.setCursor(10, 32);
    display.print("Check network.");
    display.display();
    delay(2000);
  }

  // Start NTP
  timeClient.begin();
  timeClient.update();
}

// ── Loop ──────────────────────────────────────────────────
void loop() {
  ensureWiFi();
  timeClient.update();

  // Get epoch and break into components
  unsigned long epoch = timeClient.getEpochTime();

  // Days since 1970-01-01
  unsigned long days  = epoch / 86400UL;
  int           tod   = epoch % 86400;

  int h   = tod / 3600;
  int m   = (tod % 3600) / 60;
  int s   = tod % 60;

  // Compute date from epoch (Gregorian)
  int year = 1970, month = 1, day = 1;
  unsigned long rem = days;
  while (true) {
    bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
    unsigned long yd = leap ? 366 : 365;
    if (rem < yd) break;
    rem -= yd;
    year++;
  }
  int mdays[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  bool leap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
  if (leap) mdays[1] = 29;
  for (month = 1; month <= 12; month++) {
    if ((int)rem < mdays[month - 1]) { day = rem + 1; break; }
    rem -= mdays[month - 1];
  }

  int wday = (days + 4) % 7;  // 0=Sun, Jan 1 1970 was Thursday(4)

  // ── Simulate steps (replace with real pedometer read) ──
  // stepCount = readPedometer();
  stepCount = (millis() / 5000) % STEP_GOAL;  // remove this line in production

  drawWatchFace(h, m, s, day, month, year, wday);

  delay(1000);  // refresh every second
}
