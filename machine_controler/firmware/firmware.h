#pragma once
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
///#include <QRCodeGFX.h>

// Refresh interval (in minutes) for fetching next booking info
#define NEXT_BOOKING_REFRESH_MINUTES       1
#define NEXT_BOOKING_REFRESH_INTERVAL_MS   ((unsigned long)NEXT_BOOKING_REFRESH_MINUTES * 60UL * 1000UL)

// Timezone for NTP and display (POSIX TZ string)
// Examples: "CET-1CEST,M3.5.0,M10.5.0/3" (Europe/Paris), "UTC0", "EST5EDT,M3.2.0,M11.1.0"
#define TZ_STRING   "CET-1CEST,M3.5.0,M10.5.0/3"

// Long press duration (ms)
#define LONG_PRESS_MS 700

// Book session minimum duration (minutes)
#define BOOK_SESSION_MIN_MINUTES 10

// 'preferences' keys
#define SETUP_COMPLETED_KEY     "setup_completed"
#define WIFI_AP_SSID_KEY       "wifi_ap_ssid"
#define WIFI_AP_PASS_KEY       "wifi_ap_pass"
#define WIFI_STA_SSID_KEY      "wifi_sta_ssid"
#define WIFI_STA_PASS_KEY      "wifi_sta_pass"
#define MACHINE_NAME_KEY       "machine_name"
#define UUID_KEY               "uuid"
#define MACHINE_API_HOST_KEY   "api_host"  /* NVS key max 15 chars */

// backend port (host from setup form)
#define MACHINE_API_PORT       4080
#define MACHINE_TYPE           "fm-bv2"

// HTTP client timeout (ms) for API calls;
#define MACHINE_API_TIMEOUT_MS 30000

// WiFi check and reconnect (ms)
#define WIFI_CHECK_INTERVAL_MS   2000
#define WIFI_RECONNECT_INTERVAL_MS 5000
