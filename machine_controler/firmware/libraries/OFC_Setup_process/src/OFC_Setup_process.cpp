#include "OFC_Setup_process.h"
#include "../../../preference_keys.h"
#include "../../../firmware.h"
#include "Preferences.h"
#include <esp_random.h>
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

OFC_Setup_process::OFC_Setup_process()
    : _ui(nullptr),
      _pref(nullptr),
      _dnsServer(),
      _server(80),
      _form_data{} {}

static const char ALNUM[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
#define ALNUM_LEN 62

void OFC_Setup_process::generate_ap_credentials(char* ssid, size_t ssid_max, char* pass, size_t pass_len) {
    randomSeed(esp_random());
    snprintf(ssid, ssid_max, "ofc_machine_controler_%04u", (unsigned)random(1000, 10000));
    for (size_t i = 0; i < pass_len; i++)
        pass[i] = ALNUM[esp_random() % ALNUM_LEN];
    pass[pass_len] = '\0';
}

void OFC_Setup_process::generate_uuid(char* out, size_t cap) {
    randomSeed(esp_random());
    uint32_t a = (uint32_t)esp_random();
    uint16_t b = (uint16_t)(esp_random() & 0xFFFF);
    uint16_t c = (uint16_t)((esp_random() & 0x0FFF) | 0x4000);
    uint16_t d = (uint16_t)(esp_random() & 0xFFFF);
    uint32_t e = (uint32_t)esp_random();
    uint16_t f = (uint16_t)(esp_random() & 0xFFFF);
    snprintf(out, cap, "%08lx-%04x-%04x-%04x-%08lx%04x", (unsigned long)a, b, c, d, (unsigned long)e, f);
}

void OFC_Setup_process::show_error_and_restart(const char* msg) {
    if (_ui) _ui->show_setup_error_and_restart(msg);
}

static const char SETUP_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta name="viewport" content="width=device-width,initial-scale=1"/><title>OFC Setup</title></head><body>
<h1>Machine Setup</h1>
<form method="POST" action="/">
<label>Machine name: <input name="machine_name" required maxlength="32"/></label><br/>
<label>WiFi SSID: <input name="ssid" required maxlength="32"/></label><br/>
<label>WiFi Password: <input name="password" type="password" maxlength="64"/></label><br/>
<label>Server host (IP or name): <input name="machine_api_host" required maxlength="128" placeholder="192.168.1.10"/></label><br/>
<button type="submit">Submit</button>
</form></body></html>
)rawliteral";

bool OFC_Setup_process::start_ap(char* ap_ssid, char* ap_pass) {
    generate_ap_credentials(ap_ssid, 32, ap_pass, 8);
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ap_ssid, ap_pass)) {
        Serial.println("Setup: WiFi.softAP failed");
        return false;
    }
    if (_pref) {
        _pref->putString(WIFI_AP_SSID_KEY, ap_ssid);
        _pref->putString(WIFI_AP_PASS_KEY, ap_pass);
    }
    return true;
}

void OFC_Setup_process::display_ap_instructions(const char* ap_ssid, const char* ap_pass) {
    if (_ui) _ui->show_setup_ap_instructions(ap_ssid, ap_pass);
}

void OFC_Setup_process::run_captive_portal_until_form() {
    bool form_received = false;

    _server.on("/", HTTP_GET, [this]() {
        _server.send(200, "text/html", SETUP_HTML);
    });
    _server.on("/", HTTP_POST, [this, &form_received]() {
        if (_server.hasArg("machine_name")) _server.arg("machine_name").toCharArray(_form_data.machine_name, sizeof(_form_data.machine_name));
        else _form_data.machine_name[0] = '\0';
        if (_server.hasArg("ssid")) _server.arg("ssid").toCharArray(_form_data.ssid, sizeof(_form_data.ssid));
        else _form_data.ssid[0] = '\0';
        if (_server.hasArg("password")) _server.arg("password").toCharArray(_form_data.password, sizeof(_form_data.password));
        else _form_data.password[0] = '\0';
        if (_server.hasArg("machine_api_host")) _server.arg("machine_api_host").toCharArray(_form_data.api_host, sizeof(_form_data.api_host));
        else _form_data.api_host[0] = '\0';
        form_received = true;
        _server.send(200, "text/html", "<p>Submitted. Device is connecting...</p>");
    });
    IPAddress apIP = WiFi.softAPIP();
    _server.onNotFound([this, &apIP]() {
        _server.sendHeader("Location", "http://" + apIP.toString(), true);
        _server.send(302, "text/plain", "");
    });

    _dnsServer.start(53, "*", WiFi.softAPIP());
    _server.begin();

    while (!form_received) {
        _dnsServer.processNextRequest();
        _server.handleClient();
        delay(10);
    }
}

void OFC_Setup_process::connect_to_sta_wifi() {
    if (_ui) _ui->show_setup_connecting_wifi(_form_data.ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(_form_data.ssid, _form_data.password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 20000)
        delay(200);

    if (WiFi.status() != WL_CONNECTED) {
        show_error_and_restart("WiFi connection failed");
        return;
    }
    if (_pref) {
        _pref->putString(WIFI_STA_SSID_KEY, _form_data.ssid);
        _pref->putString(WIFI_STA_PASS_KEY, _form_data.password);
    }
}

int OFC_Setup_process::register_machine_to_api() {
    if (_ui) _ui->show_setup_registering();

    char uuid[40];
    String stored = _pref ? _pref->getString(UUID_KEY, "") : String("");
    if (stored.length() == 0) {
        generate_uuid(uuid, sizeof(uuid));
        if (_pref) _pref->putString(UUID_KEY, uuid);
    } else {
        stored.toCharArray(uuid, sizeof(uuid));
    }

    String body = "{\"uuid\":\"" + String(uuid) + "\",\"name\":\"" + String(_form_data.machine_name)
        + "\",\"type\":\"" + String(MACHINE_TYPE) + "\"}";
    String url = "https://" + String(_form_data.api_host) + ":" + String(MACHINE_API_PORT) + "/machine-api/register";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);
    http.end();
    return code;
}

void OFC_Setup_process::setup_cleanup() {
    _server.stop();
    _dnsServer.stop();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void OFC_Setup_process::show_setup_complete() {
    if (_ui) _ui->show_setup_complete();
}

bool OFC_Setup_process::begin(Preferences& preferences, OFC_Ui& ui) {
    _pref = &preferences;
    _ui = &ui;

    char ap_ssid[32];
    char ap_pass[9];
    if (!start_ap(ap_ssid, ap_pass))
        return false;

    display_ap_instructions(ap_ssid, ap_pass);
    run_captive_portal_until_form();

    connect_to_sta_wifi();

    int code = register_machine_to_api();
    if (code < 200 || code >= 300) {
        show_error_and_restart("API error");
        return false;
    }

    preferences.putString(MACHINE_NAME_KEY, _form_data.machine_name);
    preferences.putString(MACHINE_API_HOST_KEY, _form_data.api_host);
    preferences.putBool(SETUP_COMPLETED_KEY, true);

    setup_cleanup();
    show_setup_complete();
    return true;
}
