#include "../OFC_Network.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

#include <cstdio>
#include <cstring>

#include "firmware.h"

// Parse RFC3339 UTC datetime (e.g. "2026-03-06T09:51:00Z") to Unix timestamp.
static int64_t rfc3339_utc_to_unix(const char* s) {
    if (!s || strlen(s) < 19) return 0;
    int y, mo, d, hh, mi, sec;
    if (sscanf(s, "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &hh, &mi, &sec) != 6) return 0;
    if (y < 1970 || mo < 1 || mo > 12 || d < 1 || d > 31) return 0;

    static const int mdays[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    int64_t days = 0;
    for (int yr = 1970; yr < y; ++yr)
        days += (yr % 4 == 0 && (yr % 100 != 0 || yr % 400 == 0)) ? 366 : 365;
    for (int m = 0; m < mo - 1; ++m)
        days += mdays[m];
    if (mo > 2 && (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)))
        days++;
    days += d - 1;

    return days * 86400LL + hh * 3600 + mi * 60 + sec;
}

// Format Unix timestamp to RFC3339 UTC (e.g. "2026-02-25T10:00:00Z").
static void unix_to_rfc3339_utc(int64_t unix_sec, char* out, size_t out_size) {
    if (!out || out_size < 21) return;
    time_t t = (time_t)unix_sec;
    struct tm tm;
    gmtime_r(&t, &tm);
    strftime(out, out_size, "%Y-%m-%dT%H:%M:%SZ", &tm);
    out[out_size - 1] = '\0';
}

void Api::set_http_error_msg(int code, char* err_msg, size_t err_size) {
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

bool Api::start_session(const char* access_key, Session* out, char* err_msg, size_t err_size) {
    if (!out) return false;
    if (_host.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "API host not configured");
        return false;
    }
    if (!access_key || access_key[0] == '\0') {
        if (err_msg && err_size) snprintf(err_msg, err_size, "access_key required");
        return false;
    }
    if (_resource_uuid.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Resource UUID missing");
        return false;
    }

    String url = "https://" + _host + "/machine-api/start_session";
    String body = "{\"access_key\":\"" + String(access_key) + "\",\"resource_uuid\":\"" + _resource_uuid + "\"}";

    _http.begin(_client, url);
    _http.addHeader("Content-Type", "application/json");
    _http.setTimeout(MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(MACHINE_API_TIMEOUT_MS);
    int code = _http.POST(body);

    if (code < 200 || code >= 300) {
        if (err_msg && err_size > 0) {
            if (code < 0) {
                set_http_error_msg(code, err_msg, err_size);
            } else {
                String payload = _http.getString();
                JsonDocument doc;
                if (!deserializeJson(doc, payload) && doc.containsKey("error")) {
                    strncpy(err_msg, doc["error"].as<const char*>(), err_size - 1);
                } else {
                    snprintf(err_msg, err_size, "HTTP %d", code);
                }
                err_msg[err_size - 1] = '\0';
            }
        }
        _http.end();
        return false;
    }

    String payload = _http.getString();
    _http.end();

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

bool Api::stop_session(char* err_msg, size_t err_size) {
    if (_host.length() == 0) return false;
    if (_resource_uuid.length() == 0) return false;

    String url = "https://" + _host + "/machine-api/stop_session";
    String body = "{\"resource_uuid\":\"" + _resource_uuid + "\"}";

    _http.begin(_client, url);
    _http.addHeader("Content-Type", "application/json");
    _http.setTimeout(MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(MACHINE_API_TIMEOUT_MS);
    int code = _http.POST(body);
    _http.end();

    if (code >= 200 && code < 300) return true;
    if (err_msg && err_size) snprintf(err_msg, err_size, "HTTP %d", code);
    return false;
}

bool Api::get_max_add_time(int* out_max, char* err_msg, size_t err_size) {
    if (!out_max) return false;
    if (_host.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "API host not configured");
        return false;
    }
    if (_resource_uuid.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Resource UUID missing");
        return false;
    }

    String url = "https://" + _host + "/machine-api/get_max_add_time";
    String body = "{\"resource_uuid\":\"" + _resource_uuid + "\"}";

    _http.begin(_client, url);
    _http.addHeader("Content-Type", "application/json");
    _http.setTimeout(MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(MACHINE_API_TIMEOUT_MS);
    int code = _http.POST(body);

    if (code < 200 || code >= 300) {
        if (err_msg && err_size > 0) {
            if (code < 0) set_http_error_msg(code, err_msg, err_size);
            else snprintf(err_msg, err_size, "HTTP %d", code);
            err_msg[err_size - 1] = '\0';
        }
        _http.end();
        return false;
    }

    String payload = _http.getString();
    _http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Invalid JSON response");
        return false;
    }
    if (!doc.containsKey("max_add_minutes")) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Missing max_add_minutes");
        return false;
    }

    *out_max = doc["max_add_minutes"].as<int>();
    return true;
}

bool Api::add_time(int add_minutes, Session* out, char* err_msg, size_t err_size) {
    if (!out) return false;
    if (add_minutes <= 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "add_minutes must be > 0");
        return false;
    }
    if (_host.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "API host not configured");
        return false;
    }
    if (_resource_uuid.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Resource UUID missing");
        return false;
    }

    String url = "https://" + _host + "/machine-api/add_time";
    String body = "{\"resource_uuid\":\"" + _resource_uuid + "\",\"add_minutes\":" + String(add_minutes) + "}";

    _http.begin(_client, url);
    _http.addHeader("Content-Type", "application/json");
    _http.setTimeout(MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(MACHINE_API_TIMEOUT_MS);
    int code = _http.POST(body);

    if (code < 200 || code >= 300) {
        if (err_msg && err_size > 0) {
            if (code < 0) {
                set_http_error_msg(code, err_msg, err_size);
            } else {
                String payload = _http.getString();
                JsonDocument doc;
                if (!deserializeJson(doc, payload) && doc.containsKey("error")) {
                    strncpy(err_msg, doc["error"].as<const char*>(), err_size - 1);
                } else {
                    snprintf(err_msg, err_size, "HTTP %d", code);
                }
                err_msg[err_size - 1] = '\0';
            }
        }
        _http.end();
        return false;
    }

    String payload = _http.getString();
    _http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Invalid JSON response");
        return false;
    }
    if (!doc.containsKey("session")) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Missing session");
        return false;
    }

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

bool Api::create_session(const char* access_key, int duration_minutes, Session* out, char* err_msg, size_t err_size) {
    if (!out) return false;
    if (duration_minutes < BOOK_SESSION_MIN_MINUTES) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Duration must be at least %d minutes", BOOK_SESSION_MIN_MINUTES);
        return false;
    }
    if (_host.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "API host not configured");
        return false;
    }
    if (!access_key || access_key[0] == '\0') {
        if (err_msg && err_size) snprintf(err_msg, err_size, "access_key required");
        return false;
    }
    if (_resource_uuid.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Resource UUID missing");
        return false;
    }

    time_t now_sec = time(nullptr);
    int64_t start_sec = (int64_t)now_sec + 2; // backend constraint
    int64_t end_sec = start_sec + (int64_t)duration_minutes * 60;
    char started_buf[32];
    char ended_buf[32];
    unix_to_rfc3339_utc(start_sec, started_buf, sizeof(started_buf));
    unix_to_rfc3339_utc(end_sec, ended_buf, sizeof(ended_buf));

    String url = "https://" + _host + "/machine-api/create_session";
    String body = "{\"access_key\":\"" + String(access_key) + "\",\"resource_uuid\":\"" + _resource_uuid
        + "\",\"started_at\":\"" + String(started_buf) + "\",\"ended_at\":\"" + String(ended_buf) + "\"}";

    _http.begin(_client, url);
    _http.addHeader("Content-Type", "application/json");
    _http.setTimeout(MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(MACHINE_API_TIMEOUT_MS);
    int code = _http.POST(body);

    if (code != 201) {
        if (err_msg && err_size > 0) {
            if (code < 0) {
                set_http_error_msg(code, err_msg, err_size);
            } else {
                String payload = _http.getString();
                JsonDocument doc;
                if (!deserializeJson(doc, payload) && doc.containsKey("error")) {
                    strncpy(err_msg, doc["error"].as<const char*>(), err_size - 1);
                } else {
                    snprintf(err_msg, err_size, "HTTP %d", code);
                }
                err_msg[err_size - 1] = '\0';
            }
        }
        _http.end();
        return false;
    }

    String payload = _http.getString();
    _http.end();

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Invalid JSON response");
        return false;
    }
    if (!doc.containsKey("session")) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Missing session");
        return false;
    }

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

