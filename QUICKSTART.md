# ⚡ EMO Clock — Quick Start Cheatsheet

## 3 things to change in emo_clock.ino
```cpp
const char* ssid          = "YOUR_WIFI_NAME";
const char* password      = "YOUR_WIFI_PASSWORD";
const char* weatherApiKey = "YOUR_KEY_FROM_OPENWEATHERMAP";
```

## Get free weather API key
→ https://openweathermap.org → Sign Up → API Keys tab

## Termux commands (copy-paste these)
```bash
pkg update && pkg upgrade
pkg install python termux-api
pip install requests
termux-wake-lock
python notifier.py
```

## IP address
→ Shown on OLED screen for 4 seconds when ESP8266 boots
→ Paste it in notifier.py as: ESP_IP = "192.168.x.x"

## Notification permission (Android)
Settings → Apps → Special App Access → Notification Access → Enable Termux

## OLED wiring
VCC → 3.3V  |  GND → GND  |  SDA → D2  |  SCL → D1

## Timezone
IST India = 19800  |  UTC = 0  |  EST USA = -18000
