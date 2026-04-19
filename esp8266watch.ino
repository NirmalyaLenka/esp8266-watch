#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi
const char* ssid     = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";

// NTP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);

void setup() {

  Serial.begin(115200);

  Wire.begin(4,5); // D2 SDA , D1 SCL

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED Failed");
    while(true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,20);
  display.print("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  timeClient.begin();
}

void loop() {

  timeClient.update();

  display.clearDisplay();
  display.setRotation(1);

  display.setTextSize(3);
  display.setCursor(5,20);

  display.print(timeClient.getFormattedTime());

  display.setTextSize(1);
  display.setCursor(5,60);

  display.print("Smart Watch");

  display.display();

  delay(1000);
}