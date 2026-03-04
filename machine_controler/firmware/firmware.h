#pragma once
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
#include <QRCodeGFX.h>

enum Menu {
    INIT,
    SCAN_CARD,
    MACHINE_INFO,
    MACHINE_USAGE,
    CONFIRM_FINISH
};

enum Event {
    EVENT_BTN_LEFT,
    EVENT_BTN_RIGHT,
    EVENT_CARD,
    EVENT_ANY
};

// Refresh interval (in minutes) for fetching next booking info
#define NEXT_BOOKING_REFRESH_MINUTES       1
#define NEXT_BOOKING_REFRESH_INTERVAL_MS   ((unsigned long)NEXT_BOOKING_REFRESH_MINUTES * 60UL * 1000UL)

// Timezone for NTP and display (POSIX TZ string)
// Examples: "CET-1CEST,M3.5.0,M10.5.0/3" (Europe/Paris), "UTC0", "EST5EDT,M3.2.0,M11.1.0"
#define TZ_STRING   "CET-1CEST,M3.5.0,M10.5.0/3"

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

struct SetupFormData {
    char machine_name[33];
    char ssid[33];
    char password[65];
    char api_host[129];
};

struct Session {
    int id;
    int user_id;
    char resource_uuid[40];
    int64_t started_at_unix;
    int64_t ended_at_unix;
    int time_used;
    char status[16];
};

struct NextBooking {
    bool has_booking;
    int64_t start_unix;
    int64_t end_unix;
    char user_name[33];
};

extern Session current_session;
extern NextBooking next_booking;
extern unsigned long last_tick_ms;
extern unsigned long last_next_booking_refresh_ms;

// sessions.ino
bool start_session(const char* access_key, const char* resource_uuid, Session* out, char* err_msg = nullptr, size_t err_size = 0);
bool stop_session(const char* resource_uuid);
void show_session_error(const char* msg);
bool fetch_next_booking(NextBooking* out);

extern char last_scanned_access_key[32];

// select_menu
void select_menu(QRCodeGFX& qr, Menu& menu, Event button);
void update_machine_usage_times(void);
void draw_machine_usage(Menu& menu);
void draw_confirm_finish(Menu& menu);

// firmware.ino helpers
void refresh_next_booking_if_needed(void);
void force_refresh_next_booking(void);

// setup_process
bool setup_process(Preferences& preferences);
bool approved_by_admin(Preferences& preferences);