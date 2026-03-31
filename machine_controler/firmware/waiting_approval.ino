#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "firmware.h"

// External reference to global h object defined in firmware.ino
extern OFC_Hardware h;
extern OFC_Ui g_ui;

bool approved_by_admin(Preferences& preferences) {
    String host = preferences.getString(MACHINE_API_HOST_KEY, "");
    String uuid = preferences.getString(UUID_KEY, "");
    if (host.length() == 0) {
        Serial.println("KO: machine API host not found");
        return false;
    }

    String url = "https://" + host + ":" + String(MACHINE_API_PORT) + "/machine-api/check_approval_status";
    String body = "{\"uuid\":\"" + uuid + "\"}";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);

    bool approved = false;
    if (code == 404) {
        Serial.println("KO: machine not found");
        g_ui.clear_screen();
        g_ui.waiting_approval();
        while (true) {
            if (h.mcp2.digitalRead(BTN_L) == LOW || h.mcp2.digitalRead(BTN_R) == LOW) {
                preferences.clear();
                ESP.restart();
            }
            delay(100);
        }
    }
    if (code >= 200 && code < 300) {
        String payload = http.getString();
        JsonDocument doc;
        if (!deserializeJson(doc, payload) && doc.containsKey("approved")) {
            approved = doc["approved"].as<bool>();
        }
    }
    http.end();
    return approved;
}
