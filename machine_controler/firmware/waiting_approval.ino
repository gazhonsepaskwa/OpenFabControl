#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

#include "firmware.h"

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
        clear_screen();
        draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
        draw_center_background(120, 60, 60);
        printTFTcentered("Error: machine deleted", tft.color565(255, 255, 255), 2, 0, 50, 320, 30);
        printTFTcentered("Press any button to reset", tft.color565(255, 255, 255), 2, 0, 80, 320, 30);
        printTFTcentered("Contact an admin", tft.color565(255, 255, 255), 2, 0, 110, 320, 30);
        while (true) {
            if (mcp2.digitalRead(BTN_L) == LOW || mcp2.digitalRead(BTN_R) == LOW) {
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
