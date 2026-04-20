/*
 ============================================================
  EMO CLOCK — ESP8266 + 0.96" OLED
  GitHub: https://github.com/YOUR_USERNAME/emo-clock
  
  Features:
  - Portrait clock (12hr + AM/PM + date)
  - WhatsApp & call notifications via WiFi
  - Live weather from OpenWeatherMap
  - Auto WiFi reconnect + NTP time sync
  - Burn-in prevention (pixel drift)
 ============================================================
  WIRING:
    OLED VCC  → 3.3V
    OLED GND  → GND
    OLED SDA  → D2 (GPIO4)
    OLED SCL  → D1 (GPIO5)
 ============================================================
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// ─── Display ──────────────────────────────────
#define SCREEN_WIDTH   128
#define SCREEN_HEIGHT  64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ESP8266WebServer server(80);

// ─────────────────────────────────────────────
//  !! CHANGE THESE SETTINGS !!
// ─────────────────────────────────────────────
const char* ssid           = "Ggg";              // Your WiFi name
const char* password       = "00000000";          // Your WiFi password
const char* weatherApiKey  = "YOUR_API_KEY_HERE"; // From openweathermap.org
const char* weatherCity    = "Agartala";          // Your city name
const int   utcOffsetSec   = 19800;               // IST=19800, UTC=0, EST=-18000
// ─────────────────────────────────────────────

// ─── Weather ──────────────────────────────────
String weatherDesc         = "--";
String weatherTemp         = "--";
unsigned long lastWeatherFetch = 0;
#define WEATHER_INTERVAL 600000UL  // fetch every 10 min

// ─── NTP ──────────────────────────────────────
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetSec, 30000);

// ─── TimeInfo struct ──────────────────────────
struct TimeInfo {
  int  h, m, s, date, month, year, dayOfWeek;
  bool pm;
  bool valid;
};

// ─── Notifications ────────────────────────────
#define MAX_NOTIFS 5
struct Notification {
  String type;      // "whatsapp" "call" "weather"
  String sender;
  String message;
  unsigned long receivedAt;
  bool   active;
};
Notification notifQueue[MAX_NOTIFS];
int  activeNotif   = -1;
unsigned long notifShowStart = 0;
#define NOTIF_DURATION 6000UL  // ms each notif shows

// ─── Screen mode ──────────────────────────────
enum ScreenMode { WATCH, NOTIFICATION };
ScreenMode currentMode = WATCH;
unsigned long lastModeSwitch = 0;

// ─── Offline timekeeping ──────────────────────
unsigned long bootEpoch  = 0;
unsigned long bootMillis = 0;
bool          timeSynced = false;

// ─── Animation ────────────────────────────────
float watchAngle = 0;
float burnAngle  = 0;

// ─────────────────────────────────────────────
// TIME HELPERS
// ─────────────────────────────────────────────
bool isLeap(int y) { return (y%4==0&&y%100!=0)||y%400==0; }
int  monthDaysArr[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

unsigned long getCurrentEpoch() {
  if (timeSynced) return bootEpoch + (millis()-bootMillis)/1000;
  return 0;
}

void syncTime() {
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    bootEpoch  = timeClient.getEpochTime();
    bootMillis = millis();
    timeSynced = true;
    Serial.println("Time synced: " + timeClient.getFormattedTime());
  }
}

int getRawHour() {
  if (!timeSynced) return 12;
  return (getCurrentEpoch()/3600)%24;
}

TimeInfo getTime() {
  TimeInfo t;
  t.valid = timeSynced;
  if (!timeSynced) {
    t.h=12;t.m=0;t.s=0;t.pm=false;
    t.date=1;t.month=1;t.year=2024;t.dayOfWeek=0;
    return t;
  }
  unsigned long epoch = getCurrentEpoch();
  t.dayOfWeek = (epoch/86400+4)%7;
  t.s  = epoch%60;
  t.m  = (epoch/60)%60;
  t.h  = (epoch/3600)%24;
  t.pm = (t.h>=12);
  if (t.h==0)      t.h=12;
  else if (t.h>12) t.h-=12;
  unsigned long days=epoch/86400;
  t.year=1970;
  while(true){
    int dy=isLeap(t.year)?366:365;
    if(days>=(unsigned long)dy){days-=dy;t.year++;}else break;
  }
  monthDaysArr[1]=isLeap(t.year)?29:28;
  t.month=0;
  while(t.month<12){
    if(days>=(unsigned long)monthDaysArr[t.month]){days-=monthDaysArr[t.month];t.month++;}else break;
  }
  t.month++;
  t.date=days+1;
  return t;
}

// ─────────────────────────────────────────────
// NOTIFICATION HELPERS
// ─────────────────────────────────────────────
void addNotification(String type, String sender, String message) {
  for (int i=0; i<MAX_NOTIFS; i++) {
    if (!notifQueue[i].active) {
      notifQueue[i] = {type, sender, message, millis(), true};
      Serial.println("[NOTIF] " + type + " from " + sender + ": " + message);
      return;
    }
  }
  // Queue full — overwrite slot 0
  notifQueue[0] = {type, sender, message, millis(), true};
}

int getNextNotif() {
  for (int i=0; i<MAX_NOTIFS; i++)
    if (notifQueue[i].active) return i;
  return -1;
}

// ─────────────────────────────────────────────
// WEB SERVER HANDLERS
// ─────────────────────────────────────────────
void handleWhatsApp() {
  if (server.hasArg("sender") && server.hasArg("message")) {
    addNotification("whatsapp", server.arg("sender"), server.arg("message"));
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing sender or message");
  }
}

void handleCall() {
  if (server.hasArg("caller")) {
    addNotification("call", server.arg("caller"), "Incoming Call");
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Missing caller");
  }
}

void handleStatus() {
  server.send(200, "text/plain",
    "EMO Clock OK\n"
    "IP: " + WiFi.localIP().toString() + "\n"
    "Time: " + (timeSynced ? timeClient.getFormattedTime() : "Not synced") + "\n"
    "Weather: " + weatherTemp + "C " + weatherDesc
  );
}

void handleRoot() {
  server.send(200, "text/html",
    "<h2>EMO Clock</h2>"
    "<p>IP: " + WiFi.localIP().toString() + "</p>"
    "<p>Endpoints:</p>"
    "<ul>"
    "<li>GET /whatsapp?sender=NAME&message=TEXT</li>"
    "<li>GET /call?caller=NAME</li>"
    "<li>GET /status</li>"
    "</ul>"
  );
}

// ─────────────────────────────────────────────
// WEATHER FETCH
// ─────────────────────────────────────────────
void fetchWeather() {
  if (WiFi.status() != WL_CONNECTED) return;
  WiFiClient client;
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q="
               + String(weatherCity)
               + "&appid=" + String(weatherApiKey)
               + "&units=metric";
  http.begin(client, url);
  int code = http.GET();
  if (code == 200) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, http.getString());
    weatherTemp = String((int)doc["main"]["temp"].as<float>());
    String raw  = doc["weather"][0]["main"].as<String>();
    if      (raw=="Thunderstorm") weatherDesc="Storm";
    else if (raw=="Drizzle")      weatherDesc="Drizzle";
    else if (raw=="Rain")         weatherDesc="Rain";
    else if (raw=="Snow")         weatherDesc="Snow";
    else if (raw=="Clear")        weatherDesc="Clear";
    else if (raw=="Clouds")       weatherDesc="Cloudy";
    else                           weatherDesc="Haze";
    addNotification("weather", weatherCity, weatherTemp+"C  "+weatherDesc);
    Serial.println("[WEATHER] "+weatherTemp+"C "+weatherDesc);
  } else {
    Serial.println("[WEATHER] HTTP error: " + String(code));
  }
  http.end();
}

// ─────────────────────────────────────────────
// DRAW WATCH (portrait mode)
// ─────────────────────────────────────────────
void drawWatch() {
  TimeInfo t = getTime();
  watchAngle += 0.025;
  int ox = (int)(sin(watchAngle)*1.0);
  int oy = (int)(cos(watchAngle*0.7)*0.8);

  display.clearDisplay();
  display.setRotation(1); // Portrait: 64 wide × 128 tall

  display.drawRoundRect(1+ox, 1+oy, 62, 126, 5, SSD1306_WHITE);

  if (!t.valid) {
    display.setTextSize(1);
    display.setCursor(5+ox, 55+oy);
    display.print("Syncing..");
    display.display();
    display.setRotation(0);
    return;
  }

  // Hours
  char hBuf[3], mBuf[3];
  sprintf(hBuf, "%02d", t.h);
  sprintf(mBuf, "%02d", t.m);

  display.setTextSize(3);
  display.setCursor(6+ox, 8+oy);
  display.print(hBuf);

  // Blinking colon
  if (t.s%2==0) {
    display.setCursor(6+ox, 34+oy);
    display.print(":");
  }

  // Minutes
  display.setCursor(6+ox, 48+oy);
  display.print(mBuf);

  // AM/PM
  display.setTextSize(1);
  display.setCursor(46+ox, 8+oy);
  display.print(t.pm ? "PM" : "AM");

  // Vertical seconds bar
  int secH = map(t.s, 0, 59, 0, 50);
  display.drawRect(52+ox, 30+oy, 5, 50, SSD1306_WHITE);
  display.fillRect(52+ox, 80-secH+oy, 5, secH, SSD1306_WHITE);

  // Divider
  display.drawLine(4+ox, 78+oy, 58+ox, 78+oy, SSD1306_WHITE);

  // Day + Date
  const char* days[]   = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
  const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                           "Jul","Aug","Sep","Oct","Nov","Dec"};
  display.setTextSize(1);
  display.setCursor(8+ox, 83+oy);
  display.print(days[t.dayOfWeek]);
  char dateBuf[8];
  sprintf(dateBuf, "%d %s", t.date, months[t.month-1]);
  display.setCursor(8+ox, 95+oy);
  display.print(dateBuf);

  // Weather mini line
  if (weatherTemp != "--") {
    display.setCursor(4+ox, 110+oy);
    String wLine = weatherTemp+"C "+weatherDesc.substring(0,5);
    display.print(wLine);
  }

  // WiFi indicator dot
  display.fillCircle(58+ox, 120+oy,
    WiFi.status()==WL_CONNECTED ? 2 : 0, SSD1306_WHITE);
  if (WiFi.status()!=WL_CONNECTED) {
    display.setCursor(50+ox,116+oy);
    display.print("--");
  }

  display.display();
  display.setRotation(0);
}

// ─────────────────────────────────────────────
// DRAW NOTIFICATION
// ─────────────────────────────────────────────
void drawNotification(int idx) {
  Notification& n = notifQueue[idx];
  int progress = map(millis()-notifShowStart, 0, NOTIF_DURATION, 62, 0);

  display.clearDisplay();
  display.setRotation(1);
  display.drawRoundRect(0, 0, 64, 128, 4, SSD1306_WHITE);

  // ── WhatsApp ──
  if (n.type == "whatsapp") {
    display.fillRoundRect(2, 2, 16, 16, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(1);
    display.setCursor(5, 6);
    display.print("W");
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(22, 4);
    display.print("WhatsApp");
    display.setCursor(4, 24);
    display.print(n.sender.substring(0,12));
    display.drawLine(4, 35, 60, 35, SSD1306_WHITE);
    // Word-wrap message
    String msg = n.message;
    int lineY = 40;
    while (msg.length()>0 && lineY<118) {
      display.setCursor(4, lineY);
      display.print(msg.substring(0, min((int)msg.length(), 10)));
      msg = msg.substring(min((int)msg.length(), 10));
      lineY += 10;
    }
  }

  // ── Call ──
  else if (n.type == "call") {
    display.fillRoundRect(2, 2, 16, 16, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(1);
    display.setCursor(5, 6);
    display.print("C");
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(22, 4);
    display.print("Calling..");
    display.setCursor(4, 25);
    display.print(n.sender.substring(0,12));
    // Pulsing rings
    int pulse = (millis()/300)%4;
    display.fillCircle(32, 75, 5, SSD1306_WHITE);
    display.drawCircle(32, 75, 10+pulse*2, SSD1306_WHITE);
    display.drawCircle(32, 75, 16+pulse*2, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(4, 105);
    display.print("Incoming Call");
  }

  // ── Weather ──
  else if (n.type == "weather") {
    display.fillRoundRect(2, 2, 16, 16, 3, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(1);
    display.setCursor(5, 6);
    display.print("*");
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(22, 4);
    display.print("Weather");
    display.setCursor(4, 22);
    display.print(weatherCity);
    display.drawLine(4, 32, 60, 32, SSD1306_WHITE);
    display.setTextSize(3);
    display.setCursor(4, 38);
    display.print(weatherTemp);
    display.setTextSize(1);
    display.print("c");
    display.setTextSize(1);
    display.setCursor(4, 72);
    display.print(weatherDesc);

    // Weather art
    if (weatherDesc=="Clear") {
      display.fillCircle(32,100,7,SSD1306_WHITE);
      display.drawLine(32,89,32,85,SSD1306_WHITE);
      display.drawLine(32,115,32,119,SSD1306_WHITE);
      display.drawLine(19,100,15,100,SSD1306_WHITE);
      display.drawLine(45,100,49,100,SSD1306_WHITE);
      display.drawLine(23,92,20,89,SSD1306_WHITE);
      display.drawLine(41,92,44,89,SSD1306_WHITE);
      display.drawLine(23,108,20,111,SSD1306_WHITE);
      display.drawLine(41,108,44,111,SSD1306_WHITE);
    } else if (weatherDesc=="Rain"||weatherDesc=="Drizzle") {
      display.fillRoundRect(14,85,36,12,5,SSD1306_WHITE);
      display.fillCircle(22,85,7,SSD1306_WHITE);
      display.fillCircle(36,85,6,SSD1306_WHITE);
      for(int d=0;d<5;d++) display.drawLine(16+d*7,100,14+d*7,108,SSD1306_WHITE);
    } else if (weatherDesc=="Cloudy") {
      display.fillRoundRect(12,90,40,12,6,SSD1306_WHITE);
      display.fillCircle(22,90,9,SSD1306_WHITE);
      display.fillCircle(36,90,7,SSD1306_WHITE);
    } else if (weatherDesc=="Storm") {
      display.fillRoundRect(12,83,40,12,6,SSD1306_WHITE);
      display.fillCircle(22,83,9,SSD1306_WHITE);
      display.fillCircle(36,83,7,SSD1306_WHITE);
      display.drawLine(34,95,27,108,SSD1306_WHITE);
      display.drawLine(27,108,32,108,SSD1306_WHITE);
      display.drawLine(32,108,25,120,SSD1306_WHITE);
    } else {
      for(int l=0;l<4;l++) display.drawLine(8,90+l*8,56,90+l*8,SSD1306_WHITE);
    }
  }

  // Progress bar
  if (progress > 0)
    display.fillRect(2, 124, progress, 2, SSD1306_WHITE);

  display.display();
  display.setRotation(0);
}

// ─────────────────────────────────────────────
// SETUP
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== EMO Clock Starting ===");

  // Init OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED not found! Check wiring.");
    while (true);
  }
  display.setTextColor(SSD1306_WHITE);

  // Init notification slots
  for (int i=0; i<MAX_NOTIFS; i++) notifQueue[i].active = false;

  // Boot screen
  display.clearDisplay();
  display.setRotation(1);
  display.setTextSize(1);
  display.setCursor(5, 30);
  display.print("EMO Clock");
  display.setCursor(5, 45);
  display.print("Starting..");
  display.display();
  display.setRotation(0);

  // Connect WiFi
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int tries = 0;
  while (WiFi.status()!=WL_CONNECTED && tries<40) {
    delay(300); tries++;
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status()==WL_CONNECTED) {
    Serial.println("WiFi connected! IP: " + WiFi.localIP().toString());
    timeClient.begin();
    syncTime();
    fetchWeather();

    // Web server routes
    server.on("/",         handleRoot);
    server.on("/whatsapp", handleWhatsApp);
    server.on("/call",     handleCall);
    server.on("/status",   handleStatus);
    server.begin();
    Serial.println("Web server started on port 80");

    // Show IP on OLED
    display.clearDisplay();
    display.setRotation(1);
    display.setTextSize(1);
    display.setCursor(4, 25);
    display.print("Connected!");
    display.setCursor(4, 40);
    display.print("IP:");
    display.setCursor(4, 52);
    display.print(WiFi.localIP().toString());
    display.setCursor(4, 70);
    display.print("Write this IP");
    display.setCursor(4, 82);
    display.print("in notifier.py");
    display.display();
    display.setRotation(0);
    delay(4000); // show IP for 4 seconds

  } else {
    Serial.println("WiFi FAILED. Check ssid/password.");
    display.clearDisplay();
    display.setRotation(1);
    display.setTextSize(1);
    display.setCursor(4, 35);
    display.print("WiFi Failed!");
    display.setCursor(4, 50);
    display.print("Check config");
    display.display();
    display.setRotation(0);
    delay(2000);
  }

  currentMode    = WATCH;
  lastModeSwitch = millis();
}

// ─────────────────────────────────────────────
// MAIN LOOP
// ─────────────────────────────────────────────
void loop() {
  unsigned long now = millis();
  server.handleClient();

  // WiFi watchdog every 15s
  static unsigned long lastWifiCheck = 0;
  if (now-lastWifiCheck > 15000) {
    lastWifiCheck = now;
    if (WiFi.status()!=WL_CONNECTED) {
      Serial.println("WiFi lost. Reconnecting...");
      WiFi.begin(ssid, password);
    } else {
      syncTime();
    }
  }

  // Weather refresh every 10 min
  if (now-lastWeatherFetch > WEATHER_INTERVAL) {
    lastWeatherFetch = now;
    fetchWeather();
  }

  // Check for new notifications
  int pending = getNextNotif();
  if (pending!=-1 && currentMode!=NOTIFICATION) {
    currentMode    = NOTIFICATION;
    activeNotif    = pending;
    notifShowStart = now;
  }

  // Notification display logic
  if (currentMode==NOTIFICATION) {
    if (activeNotif!=-1) {
      if (now-notifShowStart > NOTIF_DURATION) {
        notifQueue[activeNotif].active = false;
        activeNotif = getNextNotif();
        if (activeNotif!=-1) {
          notifShowStart = now;
        } else {
          currentMode    = WATCH;
          lastModeSwitch = now;
        }
      } else {
        drawNotification(activeNotif);
        delay(40);
        return;
      }
    }
  }

  drawWatch();
  delay(80);
}
