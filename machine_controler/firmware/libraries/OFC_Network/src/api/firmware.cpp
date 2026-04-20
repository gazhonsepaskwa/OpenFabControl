#include "../OFC_Network.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

#include <cstring>

#include "../OFC_NetworkConfig.h"

bool Api::get_last_firmware_version(char* out_version, size_t out_size, char* err_msg, size_t err_size) {
    if (!out_version || out_size == 0) return false;
    out_version[0] = '\0';

    if (_host.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "API host not configured");
        return false;
    }

    String url = "https://" + _host + "/machine-api/last-firmware-version";

    _http.begin(_client, url);
    _http.setTimeout(OFC_MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(OFC_MACHINE_API_TIMEOUT_MS);
    int code = _http.GET();

    if (code < 200 || code >= 300) {
        if (err_msg && err_size > 0) {
            if (code < 0) {
                set_http_error_msg(code, err_msg, err_size);
            } else {
                snprintf(err_msg, err_size, "HTTP %d", code);
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
        if (err_msg && err_size) snprintf(err_msg, err_size, "Invalid JSON");
        return false;
    }

    if (!doc.containsKey("version")) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Missing 'version'");
        return false;
    }

    const char* v = doc["version"].as<const char*>();
    if (!v || v[0] == '\0') {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Empty 'version'");
        return false;
    }

    strncpy(out_version, v, out_size - 1);
    out_version[out_size - 1] = '\0';
    return true;
}

bool Api::get_firmware_checksum_sha256(char* out_sha256_hex, size_t out_size, char* err_msg, size_t err_size) {
    if (!out_sha256_hex || out_size == 0) return false;
    out_sha256_hex[0] = '\0';

    if (_host.length() == 0) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "API host not configured");
        return false;
    }

    String url = "https://" + _host + "/machine-api/firmware-checksum";

    _http.begin(_client, url);
    _http.setTimeout(OFC_MACHINE_API_TIMEOUT_MS);
    _http.setConnectTimeout(OFC_MACHINE_API_TIMEOUT_MS);
    int code = _http.GET();

    if (code < 200 || code >= 300) {
        if (err_msg && err_size > 0) {
            if (code < 0) {
                set_http_error_msg(code, err_msg, err_size);
            } else {
                snprintf(err_msg, err_size, "HTTP %d", code);
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
        if (err_msg && err_size) snprintf(err_msg, err_size, "Invalid JSON");
        return false;
    }

    if (!doc.containsKey("sha256")) {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Missing 'sha256'");
        return false;
    }

    const char* h = doc["sha256"].as<const char*>();
    if (!h || h[0] == '\0') {
        if (err_msg && err_size) snprintf(err_msg, err_size, "Empty 'sha256'");
        return false;
    }

    strncpy(out_sha256_hex, h, out_size - 1);
    out_sha256_hex[out_size - 1] = '\0';
    return true;
}

