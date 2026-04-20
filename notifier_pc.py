#!/usr/bin/env python3
"""
EMO Clock Notifier — Windows PC Version
========================================
Monitors WhatsApp Desktop window title for
new messages and sends them to ESP8266 clock.

Requirements:
    pip install requests pywin32

Usage:
    python notifier_pc.py
"""

import requests
import time
import re
import sys

try:
    import win32gui
except ImportError:
    print("Missing library! Run: pip install pywin32")
    sys.exit(1)

# ─────────────────────────────────────────────
#  !! CHANGE THIS TO YOUR ESP8266 IP !!
# ─────────────────────────────────────────────
ESP_IP   = "192.168.x.x"
ESP_PORT = 80
# ─────────────────────────────────────────────

def send_whatsapp(sender, message):
    try:
        r = requests.get(
            f"http://{ESP_IP}:{ESP_PORT}/whatsapp",
            params={"sender": sender, "message": message},
            timeout=3
        )
        print(f"  ✓ Sent WA from {sender}")
    except Exception as e:
        print(f"  ✗ Error: {e}")

def get_window_titles():
    titles = []
    def callback(hwnd, _):
        if win32gui.IsWindowVisible(hwnd):
            t = win32gui.GetWindowText(hwnd)
            if t: titles.append(t)
    win32gui.EnumWindows(callback, None)
    return titles

last_wa_title = ""

def check_whatsapp():
    global last_wa_title
    for title in get_window_titles():
        if "WhatsApp" in title and title != last_wa_title:
            last_wa_title = title
            # Format: "(3) Name - WhatsApp"
            match = re.search(r'\((\d+)\)\s(.+?)\s[-|]\sWhatsApp', title)
            if match:
                count  = match.group(1)
                sender = match.group(2)
                print(f"\n[WhatsApp] {sender}: {count} new message(s)")
                send_whatsapp(sender, f"{count} new message(s)")

def main():
    print("=" * 45)
    print("  EMO Clock Notifier — Windows PC")
    print("=" * 45)

    if ESP_IP == "192.168.x.x":
        print("\n⚠️  Set your ESP_IP first!")
        sys.exit(1)

    print(f"\nMonitoring WhatsApp Desktop...")
    print("Keep WhatsApp Web/Desktop open!")
    print("Press Ctrl+C to stop\n")

    while True:
        try:
            check_whatsapp()
            time.sleep(2)
        except KeyboardInterrupt:
            print("\nStopped.")
            sys.exit(0)

if __name__ == "__main__":
    main()
