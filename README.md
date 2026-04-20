# 🤖 EMO Clock — ESP8266 + OLED Smart Watch

A smart watch built with ESP8266 and a 0.96" OLED display.  
Shows time, date, weather, and WhatsApp + call notifications — all on a tiny screen!

![EMO Clock](images/preview.png)

---

## ✨ Features

| Feature | Details |
|---------|---------|
| ⏰ Clock | 12-hour format, AM/PM, blinking colon |
| 📅 Date | Day name + date + month |
| 🌤 Weather | Live from OpenWeatherMap, updates every 10 min |
| 💬 WhatsApp | Shows sender name + message |
| 📞 Calls | Shows caller name with pulsing animation |
| 📶 WiFi | Auto reconnects if signal drops |
| 🛡 Burn-in | Subtle pixel drift prevents OLED burn |

---

## 🛒 Parts You Need

| Part | Where to Buy |
|------|-------------|
| ESP8266 NodeMCU v3 | Amazon / AliExpress |
| 0.96" OLED SSD1306 (I2C) | Amazon / AliExpress |
| Jumper wires (4x) | Amazon / AliExpress |
| Li-ion 3.7V battery (optional) | Local electronics shop |
| TP4056 charging module (optional) | AliExpress |

**Total cost: ~₹200–400**

---

## 🔌 Wiring

```
OLED Pin  →  NodeMCU Pin
────────────────────────
VCC       →  3.3V
GND       →  GND
SDA       →  D2 (GPIO4)
SCL       →  D1 (GPIO5)
```

> ⚠️ Use **3.3V** not 5V — 5V will damage the OLED!

---

## 💻 Software Setup (Step by Step)

### Step 1 — Install Arduino IDE
1. Go to https://www.arduino.cc/en/software
2. Download and install Arduino IDE 2.x
3. Open Arduino IDE

### Step 2 — Add ESP8266 Board Support
1. Open Arduino IDE
2. Go to **File → Preferences**
3. In "Additional Board Manager URLs" paste:
   ```
   https://arduino.esp8266.com/stable/package_esp8266com_index.json
   ```
4. Click OK
5. Go to **Tools → Board → Boards Manager**
6. Search `esp8266`
7. Install **esp8266 by ESP8266 Community**

### Step 3 — Install Libraries
Go to **Sketch → Include Library → Manage Libraries** and install:

| Library Name | Author |
|-------------|--------|
| `Adafruit SSD1306` | Adafruit (click "Install All" when asked) |
| `Adafruit GFX Library` | Adafruit |
| `NTPClient` | Fabrice Weinberg |
| `ArduinoJson` | Benoit Blanchon |

### Step 4 — Get Free Weather API Key
1. Go to https://openweathermap.org
2. Click **Sign Up** (free)
3. After login, go to **API Keys** tab
4. Copy your API key

### Step 5 — Configure the Code
1. Open `arduino/emo_clock/emo_clock.ino` in Arduino IDE
2. Find the settings section near the top:
   ```cpp
   const char* ssid          = "Ggg";              // ← Your WiFi name
   const char* password      = "00000000";          // ← Your WiFi password
   const char* weatherApiKey = "YOUR_API_KEY_HERE"; // ← Paste API key here
   const char* weatherCity   = "Agartala";          // ← Your city
   const int   utcOffsetSec  = 19800;               // ← IST timezone
   ```
3. Change these values to match your setup

**Timezone offsets:**
| Zone | Value |
|------|-------|
| IST (India) | `19800` |
| UTC | `0` |
| EST (USA East) | `-18000` |
| PST (USA West) | `-28800` |
| CET (Europe) | `3600` |

### Step 6 — Upload to ESP8266
1. Connect ESP8266 to PC via USB cable
2. In Arduino IDE go to **Tools → Board → ESP8266 Boards → NodeMCU 1.0**
3. Go to **Tools → Port** → select the COM port (e.g. COM3 or /dev/ttyUSB0)
4. Click the **Upload** button (→ arrow)
5. Wait for "Done uploading"
6. The OLED will show your IP address — **write it down!**

---

## 📱 Android Setup (Termux) — For Notifications

### Step 1 — Install Termux
> ⚠️ Do NOT install from Play Store — use F-Droid!

1. On your Android phone, go to: https://f-droid.org
2. Download and install **F-Droid** app
3. Open F-Droid, search **Termux** → Install
4. Also search **Termux:API** → Install

### Step 2 — Install Python in Termux
Open Termux and type these commands one by one:
```bash
pkg update
pkg upgrade
pkg install python
pip install requests
pkg install termux-api
```

### Step 3 — Allow Notification Access
1. Go to Android **Settings → Apps → Special App Access → Notification Access**
2. Enable **Termux** and **Termux:API**

### Step 4 — Copy the Script to Termux
```bash
cd ~
mkdir emo-clock
cd emo-clock
```

Now copy `python/notifier.py` to your phone (use any file manager or type it):
```bash
nano notifier.py
```
Paste the contents of `python/notifier.py`, then press `Ctrl+X → Y → Enter`

### Step 5 — Set Your ESP8266 IP
Edit the script:
```bash
nano notifier.py
```
Find this line:
```python
ESP_IP = "192.168.x.x"
```
Replace `192.168.x.x` with the IP shown on your OLED screen.

### Step 6 — Run It!
```bash
termux-wake-lock
python notifier.py
```

- `termux-wake-lock` keeps it running when screen is off
- You should see: `✓ ESP8266 reached!`
- Minimize Termux — it runs in background!

---

## 🪟 Windows PC Setup — For Notifications

```bash
pip install requests pywin32
python python/notifier_pc.py
```

- Open **WhatsApp Desktop** or **WhatsApp Web** in browser
- New messages automatically appear on the clock

---

## 🔋 Battery (Optional)

To run without USB:

```
[Li-ion 3.7V] → [TP4056 module] → [NodeMCU VIN pin]
```

| Component | Purpose |
|-----------|---------|
| Li-ion 18650 | Power source |
| TP4056 | Safe charging via USB |
| NodeMCU VIN | Has built-in 3.3V regulator |

**Battery life estimate:**
- Normal: ~25 hours (2000mAh battery)
- Deep sleep mode: months

---

## ❓ Troubleshooting

| Problem | Solution |
|---------|---------|
| OLED not showing anything | Check wiring — SDA=D2, SCL=D1 |
| "OLED not found" in Serial | Try changing `SCREEN_ADDRESS` to `0x3D` |
| WiFi not connecting | Check ssid/password in code |
| Wrong time | Check `utcOffsetSec` for your timezone |
| Weather shows `--` | Check API key, wait 10 min after signup |
| Termux can't find ESP | Make sure phone and ESP are on **same WiFi** |
| `termux-notification-list` fails | Install Termux:API + give notification permission |

---

## 📁 Project Structure

```
emo-clock/
├── arduino/
│   └── emo_clock/
│       └── emo_clock.ino     ← Upload this to ESP8266
├── python/
│   ├── notifier.py           ← Run on Android (Termux)
│   └── notifier_pc.py        ← Run on Windows PC
├── images/
│   └── preview.png
└── README.md
```

---

## 🙏 Credits & Libraries Used

- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [Adafruit GFX](https://github.com/adafruit/Adafruit-GFX-Library)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)
- [ArduinoJson](https://arduinojson.org)
- [OpenWeatherMap API](https://openweathermap.org)
- [ESP8266 Arduino Core](https://github.com/esp8266/Arduino)

---

## 📜 License

MIT License — free to use, modify, and share!
