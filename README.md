# ESP8266 Smart Watch (Minimal Clock Version) 

A **minimal WiFi Smartwatch** built using **ESP8266 Wemos D1 Mini + 0.96" OLED (SSD1306)**
This version includes **only the watch clock** for stability and testing.

---

# Features

* WiFi Time Sync (NTP)
* Clean Digital Clock UI
* Lightweight & Fast
* Stable Base Firmware

---

# Hardware Required

* ESP8266 Wemos D1 Mini
* 0.96" OLED Display (SSD1306 I2C)
* Jumper wires
* Battery (optional)

---

# Wiring Connections

| OLED | ESP8266    |
| ---- | ---------- |
| VCC  | 3.3V       |
| GND  | GND        |
| SDA  | D2 (GPIO4) |
| SCL  | D1 (GPIO5) |

 Important
Some boards don't support `D1` / `D2` labels.
Use GPIO numbers instead:

```
SDA → GPIO4
SCL → GPIO5
```

---

# Required Libraries

Install from Arduino Library Manager:

* Adafruit GFX
* Adafruit SSD1306
* ESP8266WiFi
* NTPClient

---

# Setup

Edit WiFi credentials:

```
const char* ssid     = "YOUR_WIFI";
const char* password = "YOUR_PASSWORD";
```

Upload code to ESP8266.

---

# First Boot

On startup:

1. ESP connects to WiFi
2. Gets time from internet
3. Displays clock

---

# Common Issues & Fixes

## OLED Not Working

Check:

* Correct I2C wiring
* Address (0x3C)
* Power (3.3V only)

If still not working:

Try I2C scanner sketch.

---

## ESP Not Connecting to WiFi

Check:

* Correct WiFi name
* Correct password
* 2.4GHz WiFi (ESP8266 doesn't support 5GHz)

---

## Time Not Updating

Check:

* Internet connection
* NTP server blocked
* Restart ESP

---

## Screen Blank

Check:

* Wiring
* I2C pins
* OLED voltage

---

# Performance Notes

This version is:

* Very lightweight
* Fast boot
* Stable base for future features

---

# Future Upgrade Ideas

* Notifications
* Weather
* Battery Indicator
* Vibration Motor
* Touch / Buttons

---

# Author

Nirmalya Lenka

GitHub
https://github.com/NirmalyaLenka

---

# License

MIT License
