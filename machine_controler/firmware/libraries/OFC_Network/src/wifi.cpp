#include "OFC_Network.h"
#include <WiFi.h>
#include "firmware.h"

extern Preferences preferences;
extern bool wifi_connection_lost;

void OFC_Network::connectToWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(this->SSID.c_str(), this->password.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 20000)
        delay(200);
}

bool OFC_Network::isWifiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void OFC_Network::setTimezone() {
    configTime(0, 0, "pool.ntp.org");
    setenv("TZ", TZ_STRING, 1);
    tzset();

    // wait for NTP sync (needed for draw_scan_card to compute has_booking_today)
    struct tm timeinfo;
    int ntp_retries = 0;
    while (!getLocalTime(&timeinfo) && ntp_retries++ < 30) {
        delay(500);
    }
}

void OFC_Network::checkWifiAndReconnect() {
    // get uptime
    unsigned long now_ms = millis();
    // check if the last check was less than the interval
    if (now_ms - this->last_wifi_check_ms < WIFI_CHECK_INTERVAL_MS)
        return;
    this->last_wifi_check_ms = now_ms;

    if (WiFi.status() != WL_CONNECTED) {
        if (!wifi_connection_lost) {
            wifi_connection_lost = true;
        }
        if (now_ms - this->last_wifi_reconnect_ms >= WIFI_RECONNECT_INTERVAL_MS) {
            this->last_wifi_reconnect_ms = now_ms;
            if (this->SSID.length() > 0) {
                WiFi.disconnect();
                WiFi.begin(this->SSID.c_str(), this->password.c_str());
            }
        }
    } else {
        if (wifi_connection_lost) {
            wifi_connection_lost = false;
        }
    }
}