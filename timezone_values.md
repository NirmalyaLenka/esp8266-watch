# Timezone Offset Values for OLED Watch

Replace the value in your .ino file:
```cpp
#define TZ_OFFSET_SEC  <value from table below>
```

---

## Africa

| Region / City                        | Timezone | Offset (sec) |
|--------------------------------------|----------|--------------|
| South Africa, Botswana, Zimbabwe     | UTC+2    | 7200         |
| Nigeria, Ghana, Cameroon (WAT)       | UTC+1    | 3600         |
| Egypt, Libya, Sudan                  | UTC+2    | 7200         |
| Kenya, Ethiopia, Tanzania (EAT)      | UTC+3    | 10800        |
| Morocco, Senegal (GMT)               | UTC+0    | 0            |
| Namibia (CAT)                        | UTC+2    | 7200         |

---

## Europe

| Region / City                        | Timezone | Offset (sec) |
|--------------------------------------|----------|--------------|
| UK, Portugal (GMT/WET)               | UTC+0    | 0            |
| UK Summer (BST)                      | UTC+1    | 3600         |
| France, Germany, Italy, Spain (CET)  | UTC+1    | 3600         |
| Central Europe Summer (CEST)         | UTC+2    | 7200         |
| Finland, Greece, Romania (EET)       | UTC+2    | 7200         |
| Eastern Europe Summer (EEST)         | UTC+3    | 10800        |
| Russia (Moscow)                      | UTC+3    | 10800        |

---

## Asia

| Region / City                        | Timezone   | Offset (sec) |
|--------------------------------------|------------|--------------|
| India (IST)                          | UTC+5:30   | 19800        |
| Pakistan (PKT)                       | UTC+5      | 18000        |
| Bangladesh (BST)                     | UTC+6      | 21600        |
| China, Philippines, Malaysia (CST)   | UTC+8      | 28800        |
| Japan, South Korea (JST/KST)         | UTC+9      | 32400        |
| Indonesia West (WIB)                 | UTC+7      | 25200        |
| Indonesia Central (WITA)             | UTC+8      | 28800        |
| Indonesia East (WIT)                 | UTC+9      | 32400        |
| Thailand, Vietnam, Cambodia          | UTC+7      | 25200        |
| Saudi Arabia, Iraq, Kuwait (AST)     | UTC+3      | 10800        |
| UAE, Oman (GST)                      | UTC+4      | 14400        |
| Iran (IRST)                          | UTC+3:30   | 12600        |
| Azerbaijan, Armenia (AZT)            | UTC+4      | 14400        |
| Sri Lanka (SLST)                     | UTC+5:30   | 19800        |
| Nepal (NPT)                          | UTC+5:45   | 20700        |
| Myanmar (MMT)                        | UTC+6:30   | 23400        |

---

## Americas

| Region / City                        | Timezone | Offset (sec) |
|--------------------------------------|----------|--------------|
| USA Eastern (EST)                    | UTC-5    | -18000       |
| USA Eastern Summer (EDT)             | UTC-4    | -14400       |
| USA Central (CST)                    | UTC-6    | -21600       |
| USA Central Summer (CDT)             | UTC-5    | -18000       |
| USA Mountain (MST)                   | UTC-7    | -25200       |
| USA Mountain Summer (MDT)            | UTC-6    | -21600       |
| USA Pacific (PST)                    | UTC-8    | -28800       |
| USA Pacific Summer (PDT)             | UTC-7    | -25200       |
| USA Alaska (AKST)                    | UTC-9    | -32400       |
| USA Hawaii (HST)                     | UTC-10   | -36000       |
| Canada Atlantic (AST)                | UTC-4    | -14400       |
| Brazil Brasilia (BRT)                | UTC-3    | -10800       |
| Argentina (ART)                      | UTC-3    | -10800       |
| Colombia, Peru (COT/PET)             | UTC-5    | -18000       |
| Chile (CLT)                          | UTC-4    | -14400       |
| Mexico City (CST)                    | UTC-6    | -21600       |

---

## Oceania

| Region / City                        | Timezone | Offset (sec) |
|--------------------------------------|----------|--------------|
| Australia Eastern (AEST)             | UTC+10   | 36000        |
| Australia Eastern Summer (AEDT)      | UTC+11   | 39600        |
| Australia Central (ACST)             | UTC+9:30 | 34200        |
| Australia Western (AWST)             | UTC+8    | 28800        |
| New Zealand (NZST)                   | UTC+12   | 43200        |
| New Zealand Summer (NZDT)            | UTC+13   | 46800        |
| Fiji (FJT)                           | UTC+12   | 43200        |

---

## Quick Reference — Most Common

| Country / City     | Offset (sec) |
|--------------------|--------------|
| South Africa       | 7200         |
| India              | 19800        |
| China / PH / MY    | 28800        |
| Japan / Korea      | 32400        |
| UAE / Dubai        | 14400        |
| UK (no DST)        | 0            |
| Germany / France   | 3600         |
| USA East (no DST)  | -18000       |
| USA West (no DST)  | -28800       |
| Brazil             | -10800       |
| Australia East     | 36000        |

---

> **Note:** Daylight Saving Time (DST) shifts the clock by +1 hour (+3600 sec).
> NTPClient does NOT auto-adjust for DST — update TZ_OFFSET_SEC manually
> when your region switches between summer and winter time.
