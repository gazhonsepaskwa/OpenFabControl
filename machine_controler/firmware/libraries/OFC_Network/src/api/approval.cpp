#include "../OFC_Network.h"

#include <ArduinoJson.h>

#include "firmware.h"

bool Api::is_approved_by_admin(int* out_http_code) {
    if (out_http_code) *out_http_code = 0;
    if (_host.length() == 0 || _resource_uuid.length() == 0) return false;

    String url = "https://" + _host + ":" + String(MACHINE_API_PORT) + "/machine-api/check_approval_status";
    String body = "{\"uuid\":\"" + _resource_uuid + "\"}";

    _http.begin(_client, url);
    _http.addHeader("Content-Type", "application/json");
    _http.setTimeout(MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(MACHINE_API_TIMEOUT_MS);
    int code = _http.POST(body);

    if (out_http_code) *out_http_code = code;

    bool approved = false;
    if (code >= 200 && code < 300) {
        String payload = _http.getString();
        JsonDocument doc;
        if (!deserializeJson(doc, payload) && doc.containsKey("approved")) {
            approved = doc["approved"].as<bool>();
        }
    }

    _http.end();
    return approved;
}

