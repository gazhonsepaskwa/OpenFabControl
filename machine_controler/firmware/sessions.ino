#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>

#include "firmware.h"
#include "screen_utils.h"

extern Adafruit_ILI9341 tft;

static int64_t rfc3339_to_unix(const char* s) {
    if (!s || strlen(s) < 19) return 0;
    int y, mo, d, h, mi, sec;
    if (sscanf(s, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &sec) != 6)
        return 0;
    struct tm tm = {};
    tm.tm_year = y - 1900;
    tm.tm_mon = mo - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = mi;
    tm.tm_sec = sec;
    return (int64_t)mktime(&tm);
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
    int code = http.POST(body);

    if (code < 200 || code >= 300) {
        String payload = http.getString();
        if (err_msg && err_size > 0) {
            JsonDocument doc;
            if (!deserializeJson(doc, payload) && doc.containsKey("error")) {
                strncpy(err_msg, doc["error"].as<const char*>(), err_size - 1);
            } else {
                snprintf(err_msg, err_size, "HTTP %d", code);
            }
            err_msg[err_size - 1] = '\0';
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
    out->started_at_unix = rfc3339_to_unix(started);
    out->ended_at_unix = rfc3339_to_unix(ended);
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

void show_session_error(const char* msg) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_center_background(120, 60, 60);
    printTFTcentered(msg ? msg : "Error", tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
}
