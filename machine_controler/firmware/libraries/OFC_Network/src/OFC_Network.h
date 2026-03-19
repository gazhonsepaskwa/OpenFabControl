#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

struct NextBooking {
    bool has_booking;
    int64_t start_unix;
    int64_t end_unix;
    char user_name[33];
};

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

class Api {
    // Constructor / Destructor
    public:
        Api(String resource_uuid, String host);
       ~Api();

    // Attributs
    private:
        // config
        String              _resource_uuid;
        String              _host;
        WiFiClientSecure    _client;
        HTTPClient          _http;

    // Privates Methodes
    private:
        bool fetchNextBooking(NextBooking* out);
        bool next_booking_equals(const NextBooking& a, const NextBooking& b);

    // Methodes
    public:
        bool refresh_next_booking_if_needed(void);
        void force_refresh_next_booking(void);
        NextBooking get_next_booking(void);

};

class OFC_Network {
    public:
        OFC_Network();
        OFC_Network(String ssid, String password, String resource_uuid, String host);
       ~OFC_Network();

        void connectToWifi();
        bool isWifiConnected();
        void setTimezone();
        void checkWifiAndReconnect();

        Api* api = nullptr;

    private:
        String SSID;
        String password;
        unsigned long last_wifi_check_ms = 0;
        unsigned long last_wifi_reconnect_ms = 0;
};
