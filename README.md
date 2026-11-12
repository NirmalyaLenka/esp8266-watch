# 🤖 EMO Clock — ESP8266 + OLED Smart Watch

A smart-watch-style clock prototype for an ESP8266 and a 128×64 SSD1306 I²C OLED. The primary sketch combines time/date, network weather, and notification display. This repository also contains three alternative watch sketches and two host-side notification scripts.

The repository contains `index.svg`, but no `images/preview.png`; the file guide below reflects the actual root-level layout.

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

Costs vary by board and display; current prices have not been checked.

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

> **Electrical note:** This wiring assumes a 3.3 V supply and compatible I²C logic. Check the datasheet/markings for your exact OLED module before powering it.

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
1. Open the root-level `emo_clock.ino` in Arduino IDE. The other `.ino` files are alternative sketches, not files inside an `arduino/` folder.
2. In the settings section near the top, replace the Wi-Fi and weather values with your own configuration:
   ```cpp
   const char* ssid          = "YOUR_WIFI_NAME";
   const char* password      = "YOUR_WIFI_PASSWORD";
   const char* weatherApiKey = "YOUR_API_KEY_HERE";
   const char* weatherCity   = "YOUR_CITY";
   const int   utcOffsetSec  = 19800; // IST; adjust for your timezone
   ```
3. Save the sketch before compiling. Do not commit real Wi-Fi passwords or API keys. Some sketches in this repository contain hard-coded network settings; replace those before use, and rotate any credentials that were real and have already been published.

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

Copy the root-level `notifier.py` to your phone (use a file manager, USB, or another transfer method):
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
python -m pip install requests pywin32
python notifier_pc.py
```

- Keep WhatsApp Desktop or WhatsApp Web open. The script reads window titles and sends a detected sender plus a message-count summary; it does **not** forward the full message body.

---

## 🔋 Battery power

Battery operation is not validated by the files in this repository. Do not assume a bare 3.7 V cell, charger board, and NodeMCU VIN connection form a safe or correctly regulated supply. Use a protected cell, a charger designed for that cell, and a power path matched to the exact development board; verify the board and charger datasheets before connecting a battery. No battery-life estimate is claimed.

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

```text
.
├── emo_clock.ino         # Primary feature-rich ESP8266 sketch
├── esp8266watch.ino      # Minimal Wi-Fi/NTP clock sketch
├── oled_watch.ino        # Alternate watch face; step count is a placeholder
├── watchfuneddition.ino  # Watch/animated-eyes sketch
├── notifier.py           # Android notification bridge (Termux)
├── notifier_pc.py        # Windows WhatsApp window-title bridge
├── QUICKSTART.md         # Short setup checklist
├── timezone_values.md    # UTC offset reference
├── index.svg             # SVG asset
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

No license file is present in the repository. Reuse and redistribution terms are therefore not explicitly specified; add a license file if you intend to publish the project for reuse.


---

## Skills & technologies

[![C++](https://img.shields.io/badge/C%2B%2B-cpp?logo=C%2B%2B&logoColor=white)](https://github.com/NirmalyaLenka/esp8266-watch) · [![Python](https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=white)](https://github.com/NirmalyaLenka/esp8266-watch)


## Sketch variants

- **`emo_clock.ino`** — primary sketch for the clock/date, Wi-Fi, NTP, weather, and HTTP notification display.
- **`esp8266watch.ino`** — minimal clock that connects to Wi-Fi and uses NTP.
- **`oled_watch.ino`** — alternate watch face; its step counter is a fixed placeholder, not a real pedometer.
- **`watchfuneddition.ino`** — animated watch/eyes experiment.

Open and upload only one sketch at a time. The primary wiring documented above applies to the SSD1306 I²C display; confirm each sketch's settings before flashing it.

## Security, privacy, and limitations

- The Android notifier reads notification metadata/content via Termux:API and sends selected fields to the ESP8266 over plain HTTP on the local network. Run it only on a network you trust and stop it with `Ctrl+C` when not needed.
- The Windows notifier polls visible window titles and sends a count summary rather than full WhatsApp message content.
- The ESP8266 HTTP endpoints are not authenticated or encrypted in the inspected sketch. Do not expose the device or port 80 directly to the public internet.
- Weather needs a valid OpenWeatherMap API key and working internet access. Time depends on NTP synchronization; verify the UTC offset for your location.
- The notification scripts require the ESP8266 IP address to be configured manually and the host and board to be able to reach each other on the same network.
- The sketches and wiring were inspected for this documentation pass, but they were not compiled or tested on hardware here.

## Maintenance & Audit

- **Audit date:** 11 November 2026
- **Documentation status:** README reviewed and audit record added on this date.
- **Scope and evidence:** This is a documentation-level maintenance pass. It does not establish that source code compiles, dependencies are current, security has been tested, or hardware/runtime behavior has been verified.
- **Code and tests:** No new functional code changes or test results are claimed by this README update. Run project-specific build, test, and hardware/runtime checks before relying on the project.
- **Maintenance baseline:** Documentation status recorded through 11 November 2026; revisit setup instructions, dependencies, known limitations, and tests when the project changes.
