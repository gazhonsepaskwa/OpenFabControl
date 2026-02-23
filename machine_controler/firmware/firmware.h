#pragma once
#include <Preferences.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
#include <QRCodeGFX.h>

enum Menu {
    INIT,
    SCAN_CARD_NO_RESERVATION,
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

extern Session current_session;
extern unsigned long last_tick_ms;

bool start_session(const char* access_key, const char* resource_uuid, Session* out, char* err_msg = nullptr, size_t err_size = 0);
bool stop_session(const char* resource_uuid);
void show_session_error(const char* msg);

extern char last_scanned_access_key[32];

void select_menu(QRCodeGFX& qr, Menu& menu, Event button);
void update_machine_usage_times(void);
bool setup_process(Preferences& preferences);
bool approved_by_admin(Preferences& preferences);
