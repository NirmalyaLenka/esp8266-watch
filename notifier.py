#!/usr/bin/env python3
"""
EMO Clock Notifier — Android (Termux) Version
==============================================
Sends WhatsApp, call, and weather notifications
to your ESP8266 clock via WiFi HTTP requests.

Requirements:
    pkg install python
    pip install requests

Usage:
    python notifier.py
"""

import requests
import time
import subprocess
import json
import sys
import os

# ─────────────────────────────────────────────
#  !! CHANGE THIS TO YOUR ESP8266 IP !!
#  (IP is shown on OLED screen when it boots)
# ─────────────────────────────────────────────
ESP_IP   = "192.168.x.x"
ESP_PORT = 80
# ─────────────────────────────────────────────

CHECK_INTERVAL = 2  # seconds between notification checks

def send_to_esp(endpoint, params):
    """Send HTTP request to ESP8266."""
    try:
        url = f"http://{ESP_IP}:{ESP_PORT}/{endpoint}"
        r   = requests.get(url, params=params, timeout=3)
        if r.status_code == 200:
            print(f"  ✓ Sent [{endpoint}] → {params}")
        else:
            print(f"  ✗ ESP responded with: {r.status_code}")
    except requests.exceptions.ConnectionError:
        print(f"  ✗ Cannot reach ESP8266 at {ESP_IP}")
        print("    → Make sure phone & ESP are on the same WiFi!")
    except requests.exceptions.Timeout:
        print(f"  ✗ ESP8266 timeout")
    except Exception as e:
        print(f"  ✗ Error: {e}")

def send_whatsapp(sender, message):
    send_to_esp("whatsapp", {"sender": sender, "message": message})

def send_call(caller):
    send_to_esp("call", {"caller": caller})

def check_esp_reachable():
    """Test connection to ESP8266 before starting."""
    print(f"\nTesting connection to ESP8266 at {ESP_IP}...")
    try:
        r = requests.get(f"http://{ESP_IP}:{ESP_PORT}/status", timeout=4)
        print(f"  ✓ ESP8266 reached! Response: {r.text.strip()}")
        return True
    except Exception:
        print(f"  ✗ Cannot reach ESP8266 at {ESP_IP}")
        print("  Possible reasons:")
        print("  1. Wrong IP — check OLED screen when ESP boots")
        print("  2. Phone and ESP are on different WiFi networks")
        print("  3. ESP8266 not powered on")
        return False

def get_android_notifications():
    """Read notifications using Termux:API."""
    try:
        result = subprocess.run(
            ["termux-notification-list"],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.returncode != 0 or not result.stdout.strip():
            return []
        return json.loads(result.stdout)
    except FileNotFoundError:
        print("  ✗ termux-notification-list not found!")
        print("    → Install Termux:API from F-Droid, then:")
        print("    → pkg install termux-api")
        return []
    except json.JSONDecodeError:
        return []
    except Exception as e:
        print(f"  ✗ Notification read error: {e}")
        return []

def process_notifications(notifs, seen):
    """Filter and forward relevant notifications to ESP8266."""
    for n in notifs:
        pkg   = n.get("packageName", "").lower()
        title = n.get("title", "") or ""
        text  = n.get("content", "") or n.get("text", "") or ""
        nid   = str(n.get("id", "0"))

        # ── WhatsApp ──
        if "whatsapp" in pkg and title:
            key = f"wa_{nid}_{title}"
            if key not in seen:
                seen.add(key)
                print(f"\n[WhatsApp] {title}: {text[:40]}")
                send_whatsapp(title, text[:50])

        # ── Phone Call ──
        call_pkgs = ["incall", "dialer", "phone", "telecom", "calling"]
        if any(c in pkg for c in call_pkgs) and title:
            key = f"call_{nid}"
            if key not in seen:
                seen.add(key)
                print(f"\n[Call] Incoming: {title}")
                send_call(title or "Unknown Number")

def main():
    print("=" * 45)
    print("  EMO Clock Notifier — Android Edition")
    print("=" * 45)

    # Validate IP
    if ESP_IP == "192.168.x.x":
        print("\n⚠️  ERROR: You haven't set your ESP8266 IP!")
        print("   1. Power on your ESP8266 clock")
        print("   2. Look at the OLED — it shows the IP")
        print("   3. Edit this file: nano notifier.py")
        print("   4. Change ESP_IP = '192.168.x.x' to your IP")
        sys.exit(1)

    # Test connection
    if not check_esp_reachable():
        print("\nContinuing anyway — will retry on each check...")

    print(f"\nMonitoring notifications every {CHECK_INTERVAL}s")
    print("Press Ctrl+C to stop\n")

    seen = set()
    loop_count = 0

    while True:
        try:
            notifs = get_android_notifications()
            process_notifications(notifs, seen)

            # Keep seen set small
            if len(seen) > 200:
                seen = set(list(seen)[-100:])

            loop_count += 1
            if loop_count % 150 == 0:  # every ~5 min
                print(f"  [Running... {loop_count*CHECK_INTERVAL//60} min]")

            time.sleep(CHECK_INTERVAL)

        except KeyboardInterrupt:
            print("\n\nStopped by user. Goodbye!")
            sys.exit(0)
        except Exception as e:
            print(f"  Loop error: {e}")
            time.sleep(5)

if __name__ == "__main__":
    main()
