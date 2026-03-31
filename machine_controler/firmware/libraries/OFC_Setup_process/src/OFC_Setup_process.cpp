#include "OFC_Setup_process.h"
#include "../../../preference_keys.h"
#include "Preferences.h"
#include <esp_random.h>
#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Constructor / Destructor
OFC_Setup_process::OFC_Setup_process(OFC_Ui* ui, Preferences* pref)
:   _ui(ui),
    _pref(pref),
    _dnsServer(),
    _server(80)) {}

OFC_Setup_process::OFC_Setup_process();

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
    _pref.putString(WIFI_AP_SSID_KEY, ap_ssid);
    _pref.putString(WIFI_AP_PASS_KEY, ap_pass);
    return true;
}

void OFC_Setup_process::display_ap_instructions(const char* ap_ssid, const char* ap_pass) {
    if (_ui) _ui->show_setup_ap_instructions(ap_ssid, ap_pass);
}

void OFC_Setup_process::run_captive_portal_until_form() {
    bool form_received = false;

   _server.on("/", HTTP_GET, [&_server]() {
       _server.send(200, "text/html", SETUP_HTML);
    });
   _server.on("/", HTTP_POST, [&_server, &form_data, &form_received]() {
        if (_server.hasArg("machine_name"))_server.arg("machine_name").toCharArray(form_data.machine_name, sizeof(form_data.machine_name));
        else form_data.machine_name[0] = '\0';
        if (_server.hasArg("ssid"))_server.arg("ssid").toCharArray(form_data.ssid, sizeof(form_data.ssid));
        else form_data.ssid[0] = '\0';
        if (_server.hasArg("password"))_server.arg("password").toCharArray(form_data.password, sizeof(form_data.password));
        else form_data.password[0] = '\0';
        if (_server.hasArg("machine_api_host"))_server.arg("machine_api_host").toCharArray(form_data.api_host, sizeof(form_data.api_host));
        else form_data.api_host[0] = '\0';
        form_received = true;
       _server.send(200, "text/html", "<p>Submitted. Device is connecting...</p>");
    });
    IPAddress apIP = WiFi.softAPIP();
   _server.onNotFound([&_server, &apIP]() {
       _server.sendHeader("Location", "http://" + apIP.toString(), true);
       _server.send(302, "text/plain", "");
    });

    dnsServer.start(53, "*", WiFi.softAPIP());
   _server.begin();

    while (!form_received) {
        dnsServer.processNextRequest();
       _server.handleClient();
        delay(10);
    }
}

void OFC_Setup_process::connect_to_sta_wifi(const SetupFormData& form_data, Preferences& preferences) {
    if (_ui) _ui->show_setup_connecting_wifi(form_data.ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(form_data.ssid, form_data.password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < 20000)
        delay(200);

    if (WiFi.status() != WL_CONNECTED) {
        show_error_and_restart("WiFi connection failed");
    }
    preferences.putString(WIFI_STA_SSID_KEY, form_data.ssid);
    preferences.putString(WIFI_STA_PASS_KEY, form_data.password);
}

int OFC_Setup_process::register_machine_to_api(const SetupFormData& form_data, Preferences& preferences) {
    if (_ui) _ui->show_setup_registering();

    char uuid[40];
    String stored = preferences.getString(UUID_KEY, "");
    if (stored.length() == 0) {
        generate_uuid(uuid, sizeof(uuid));
        preferences.putString(UUID_KEY, uuid);
    } else {
        stored.toCharArray(uuid, sizeof(uuid));
    }

    String body = "{\"uuid\":\"" + String(uuid) + "\",\"name\":\"" + String(form_data.machine_name)
        + "\",\"type\":\"" + String(MACHINE_TYPE) + "\"}";
    String url = "https://" + String(form_data.api_host) + ":" + String(MACHINE_API_PORT) + "/machine-api/register";

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(body);
    http.end();
    return code;
}

void OFC_Setup_process::setup_cleanup(WebServer& _server, DNSServer& dnsServer) {
   _server.stop();
    dnsServer.stop();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

void OFC_Setup_process::show_setup_complete(void) {
    if (_ui) _ui->show_setup_complete();
}

bool OFC_setup_process::begin(Preferences& preferences) {
    char ap_ssid[32];
    char ap_pass[9];
    if (!start_ap(preferences, ap_ssid, ap_pass))
        return false;

    display_ap_instructions(ap_ssid, ap_pass);
    run_captive_portal_until_form(_server, dnsServer, form_data);

    connect_to_sta_wifi(form_data, preferences);

    int code = register_machine_to_api(form_data, preferences);
    if (code < 200 || code >= 300) {
        show_error_and_restart("API error");
        return false;
    }

    preferences.putString(MACHINE_NAME_KEY, form_data.machine_name);
    preferences.putString(MACHINE_API_HOST_KEY, form_data.api_host);
    preferences.putBool(SETUP_COMPLETED_KEY, true);

    setup_cleanup(_server, dnsServer);
    show_setup_complete();
    return true;
}
