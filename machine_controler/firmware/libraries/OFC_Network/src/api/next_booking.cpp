#include "../OFC_Network.h"
#include <ArduinoJson.h>
#include <string.h>
#include "firmware.h"

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

// Server must return the *current* ongoing booking if any (slot already started, not yet ended),
// not only the next future one; otherwise the device shows FREE during an active booking.
bool Api::fetchNextBooking(NextBooking* out) {
    if (!out) return false;

    if (_host.length() == 0) {
        out->has_booking = false;
        return false;
    }

    if (_resource_uuid.length() == 0) {
        out->has_booking = false;
        return false;
    }

    String url = "https://" + _host + "/machine-api/next_booking"; //this->host + ":" + String(MACHINE_API_PORT)
    String body = "{\"resource_uuid\":\"" + _resource_uuid + "\"}";

    _http.begin(_client, url);
    _http.addHeader("Content-Type", "application/json");
    int code = _http.POST(body);

    if (code < 200 || code >= 300) {
        _http.end();
        out->has_booking = false;
        return false;
    }

    String payload = _http.getString();
    _http.end();

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
    const char* end_s   = nb["end_at"].as<const char*>();
    const char* user_s  = nb["user_name"].as<const char*>();

    out->start_unix = rfc3339_utc_to_unix(start_s);
    out->end_unix   = rfc3339_utc_to_unix(end_s);
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

// helper
bool Api::next_booking_equals(const NextBooking& a, const NextBooking& b) {
    if (a.has_booking != b.has_booking) return false;
    if (a.start_unix != b.start_unix || a.end_unix != b.end_unix) return false;
    return (strcmp(a.user_name, b.user_name) == 0);
}

// update the next_booking (with fetchNextBooking) attribut if last time it has been done is at least NEXT_BOOKING_REFRESH_INTERVAL_MS later
bool Api::refresh_next_booking_if_needed(void) {
    static unsigned long    last_next_booking_refresh_ms = 0;

    // exit if not the right time
    unsigned long now_ms = millis();
    if ((now_ms - last_next_booking_refresh_ms) < NEXT_BOOKING_REFRESH_INTERVAL_MS) {
        return false;
    }

    // Refresh icon only when checking for next booking (scan card screen), no full screen update
    //if (menu == SCAN_CARD) {
    //    draw_title_right_status(true);
    //}

    NextBooking fetched = {};

    // returne false if error
    if (!this->fetchNextBooking(&fetched)) {
        // if (menu == SCAN_CARD) draw_title_right_status(false);
        return false;
    }

    // update last refresh time
    last_next_booking_refresh_ms = now_ms;

    // check if something changed from last fetch
    bool changed = !next_booking_equals(_next_booking, fetched);
    _next_booking = fetched;

    return changed;
}

void Api::force_refresh_next_booking(void) {
    if (this->fetchNextBooking(&_next_booking)) {
        last_next_booking_refresh_ms = millis();
    }
}

NextBooking Api::get_next_booking(void) {
    return _next_booking;
}
