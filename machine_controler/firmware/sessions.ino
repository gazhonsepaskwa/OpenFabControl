#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "firmware.h"
#include "screen_utils.h"

extern Adafruit_ILI9341 tft;

// Parse RFC3339 UTC datetime (e.g. "2026-03-06T09:51:00Z") to Unix timestamp.
static int64_t rfc3339_utc_to_unix(const char* s) {
    if (!s || strlen(s) < 19) return 0;
    int y, mo, d, h, mi, sec;
    if (sscanf(s, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &sec) != 6)
        return 0;
    if (y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31)
        return 0;

    static const int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

    int64_t days = 0;
    for (int yr = 1970; yr < y; ++yr)
        days += (yr % 4 == 0 && (yr % 100 != 0 || yr % 400 == 0)) ? 366 : 365;
    for (int m = 0; m < mo - 1; ++m)
        days += mdays[m];
    if (mo > 2 && (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)))
        days++;
    days += d - 1;

    return days * 86400LL + h * 3600 + mi * 60 + sec;
}

// Map HTTPClient error codes to error msg
static void set_http_error_msg(int code, char* err_msg, size_t err_size) {
    if (!err_msg || err_size == 0) return;
    const char* msg = "Connection failed";
    if (code == -1) msg = "Connection refused (check server)";
    else if (code == -4) msg = "WiFi disconnected";
    else if (code == -5) msg = "Connection lost";
    else if (code == -6 || code == -11) msg = "Timeout (try again)";
    else if (code < 0) msg = "Network error";
    snprintf(err_msg, err_size, "%s", msg);
    err_msg[err_size - 1] = '\0';
}

bool start_session(const char* access_key, const char* resource_uuid, Session* out, char* err_msg, size_t err_size) {
    String host = preferences.getString(MACHINE_API_HOST_KEY, "");
    if (host.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "API host not configured");
        return false;
    }

    String url = "https://" + host + ":" + String(MACHINE_API_PORT) + "/machine-api/start_session";
    String body = "{\"access_key\":\"" + String(access_key) + "\",\"resource_uuid\":\"" + String(resource_uuid) + "\"}";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(MACHINE_API_TIMEOUT_MS);
    http.setConnectTimeout(MACHINE_API_TIMEOUT_MS);
    int code = http.POST(body);

    if (code < 200 || code >= 300) {
        if (err_msg && err_size > 0) {
            if (code < 0) {
                set_http_error_msg(code, err_msg, err_size);
            } else {
                String payload = http.getString();
                JsonDocument doc;
                if (!deserializeJson(doc, payload) && doc.containsKey("error")) {
                    strncpy(err_msg, doc["error"].as<const char*>(), err_size - 1);
                } else {
                    snprintf(err_msg, err_size, "HTTP %d", code);
                }
                err_msg[err_size - 1] = '\0';
            }
        }
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload)) return false;
    if (!doc.containsKey("session")) return false;

    JsonObject sess = doc["session"];
    out->id = sess["id"].as<int>();
    out->user_id = sess["user_id"].as<int>();
    String ru = sess["resource_uuid"].as<String>();
    strncpy(out->resource_uuid, ru.c_str(), sizeof(out->resource_uuid) - 1);
    out->resource_uuid[sizeof(out->resource_uuid) - 1] = '\0';

    const char* started = sess["started_at"].as<const char*>();
    const char* ended = sess["ended_at"].as<const char*>();
    out->started_at_unix = rfc3339_utc_to_unix(started);
    out->ended_at_unix = rfc3339_utc_to_unix(ended);
    out->time_used = sess["time_used"].as<int>();

    String st = sess["status"].as<String>();
    strncpy(out->status, st.c_str(), sizeof(out->status) - 1);
    out->status[sizeof(out->status) - 1] = '\0';

    return true;
}

bool stop_session(const char* resource_uuid) {
    String host = preferences.getString(MACHINE_API_HOST_KEY, "");
    if (host.length() == 0) return false;

    String url = "https://" + host + ":" + String(MACHINE_API_PORT) + "/machine-api/stop_session";
    String body = "{\"resource_uuid\":\"" + String(resource_uuid) + "\"}";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);
    http.end();

    return (code >= 200 && code < 300);
}

// Server must return the *current* ongoing booking if any (slot already started, not yet ended),
// not only the next future one; otherwise the device shows FREE during an active booking.
bool fetch_next_booking(NextBooking* out) {
    if (!out) return false;

    String host = preferences.getString(MACHINE_API_HOST_KEY, "");
    if (host.length() == 0) {
        out->has_booking = false;
        return false;
    }

    String resource_uuid = preferences.getString(UUID_KEY, "");
    if (resource_uuid.length() == 0) {
        out->has_booking = false;
        return false;
    }

    String url = "https://" + host + ":" + String(MACHINE_API_PORT) + "/machine-api/next_booking";
    String body = "{\"resource_uuid\":\"" + resource_uuid + "\"}";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);

    if (code < 200 || code >= 300) {
        http.end();
        out->has_booking = false;
        return false;
    }

    String payload = http.getString();
    http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        out->has_booking = false;
        return false;
    }

    if (!doc.containsKey("next_booking") || doc["next_booking"].isNull()) {
        out->has_booking = false;
        return true; // no future booking, but not an error
    }

    JsonObject nb = doc["next_booking"];

    const char* start_s = nb["start_at"].as<const char*>();
    const char* end_s = nb["end_at"].as<const char*>();
    const char* user_s = nb["user_name"].as<const char*>();

    out->start_unix = rfc3339_utc_to_unix(start_s);
    out->end_unix = rfc3339_utc_to_unix(end_s);
    if (user_s) {
        strncpy(out->user_name, user_s, sizeof(out->user_name) - 1);
        out->user_name[sizeof(out->user_name) - 1] = '\0';
    } else {
        out->user_name[0] = '\0';
    }

    if (out->start_unix <= 0 || out->end_unix <= 0 || out->end_unix <= out->start_unix) {
        out->has_booking = false;
        return true;
    }

    out->has_booking = true;
    return true;
}

void show_session_error(const char* msg) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_center_background(120, 60, 60);
    printTFTcentered(msg ? msg : "Error", tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
}
