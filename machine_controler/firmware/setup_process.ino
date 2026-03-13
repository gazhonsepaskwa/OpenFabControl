#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <esp_random.h>

#include "firmware.h"
#include "screen_utils.h"

// External reference to global h object defined in firmware.ino
extern hardware h;

static const char ALNUM[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
#define ALNUM_LEN 62

static void generate_ap_credentials(char* ssid, size_t ssid_max, char* pass, size_t pass_len) {
    randomSeed(esp_random());
    snprintf(ssid, ssid_max, "ofc_machine_controler_%04u", (unsigned)random(1000, 10000));
    for (size_t i = 0; i < pass_len; i++)
        pass[i] = ALNUM[esp_random() % ALNUM_LEN];
    pass[pass_len] = '\0';
}

static void generate_uuid(char* out, size_t cap) {
    randomSeed(esp_random());
    uint32_t a = (uint32_t)esp_random();
    uint16_t b = (uint16_t)(esp_random() & 0xFFFF);
    uint16_t c = (uint16_t)((esp_random() & 0x0FFF) | 0x4000);
    uint16_t d = (uint16_t)(esp_random() & 0xFFFF);
    uint32_t e = (uint32_t)esp_random();
    uint16_t f = (uint16_t)(esp_random() & 0xFFFF);
    snprintf(out, cap, "%08lx-%04x-%04x-%04x-%08lx%04x", (unsigned long)a, b, c, d, (unsigned long)e, f);
}

static void show_error_and_restart(const char* msg) {
    clear_screen();
    draw_title((char*)"Setup Error");
    draw_center_background(120, 60, 60);
    printTFTcentered(msg, h.tft.color565(255, 255, 255), 2, 0, 40, 320, 120);
    printTFTcentered("Restart in 5s...", h.tft.color565(255, 255, 255), 2, 0, 120, 320, 30);
    delay(5000);
    ESP.restart();
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

static bool start_ap(Preferences& preferences, char* ap_ssid, char* ap_pass) {
    generate_ap_credentials(ap_ssid, 32, ap_pass, 8);
    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(ap_ssid, ap_pass)) {
        Serial.println("Setup: WiFi.softAP failed");
        return false;
    }
    preferences.putString(WIFI_AP_SSID_KEY, ap_ssid);
    preferences.putString(WIFI_AP_PASS_KEY, ap_pass);
    return true;
}

static void display_ap_instructions(const char* ap_ssid, const char* ap_pass) {
    clear_screen();
    draw_title((char*)"Setup");
    draw_center_background(60, 60, 120);
    printTFTcentered("Connect to WiFi:", h.tft.color565(255, 255, 255), 2, 0, 25, 320, 22);
    printTFTcentered(ap_ssid, h.tft.color565(255, 255, 255), 2, 0, 47, 320, 22);
    printTFTcentered("Password:", h.tft.color565(255, 255, 255), 2, 0, 69, 320, 22);
    printTFTcentered(ap_pass, h.tft.color565(255, 255, 255), 2, 0, 91, 320, 22);
    printTFTcentered("Then open browser", h.tft.color565(255, 255, 255), 2, 0, 118, 320, 22);
    printTFTcentered("and fill the form", h.tft.color565(255, 255, 255), 2, 0, 140, 320, 22);
}

static void run_captive_portal_until_form(WebServer& server, DNSServer& dnsServer, SetupFormData& form_data) {
    bool form_received = false;

    server.on("/", HTTP_GET, [&server]() {
        server.send(200, "text/html", SETUP_HTML);
    });
    server.on("/", HTTP_POST, [&server, &form_data, &form_received]() {
        if (server.hasArg("machine_name")) server.arg("machine_name").toCharArray(form_data.machine_name, sizeof(form_data.machine_name));
        else form_data.machine_name[0] = '\0';
        if (server.hasArg("ssid")) server.arg("ssid").toCharArray(form_data.ssid, sizeof(form_data.ssid));
        else form_data.ssid[0] = '\0';
        if (server.hasArg("password")) server.arg("password").toCharArray(form_data.password, sizeof(form_data.password));
        else form_data.password[0] = '\0';
        if (server.hasArg("machine_api_host")) server.arg("machine_api_host").toCharArray(form_data.api_host, sizeof(form_data.api_host));
        else form_data.api_host[0] = '\0';
        form_received = true;
        server.send(200, "text/html", "<p>Submitted. Device is connecting...</p>");
    });
    IPAddress apIP = WiFi.softAPIP();
    server.onNotFound([&server, &apIP]() {
        server.sendHeader("Location", "http://" + apIP.toString(), true);
        server.send(302, "text/plain", "");
    });

    dnsServer.start(53, "*", WiFi.softAPIP());
    server.begin();

    while (!form_received) {
        dnsServer.processNextRequest();
        server.handleClient();
        delay(10);
    }
}

static void connect_to_sta_wifi(const SetupFormData& form_data, Preferences& preferences) {
    clear_screen();
    draw_title((char*)"Setup");
    draw_center_background(60, 60, 120);
    printTFTcentered("Connecting to WiFi...", h.tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
    printTFTcentered(form_data.ssid, h.tft.color565(200, 200, 200), 2, 0, 100, 320, 25);

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

static int register_machine_to_api(const SetupFormData& form_data, Preferences& preferences) {
    clear_screen();
    draw_title((char*)"Setup");
    draw_center_background(60, 60, 120);
    printTFTcentered("Registering...", h.tft.color565(255, 255, 255), 2, 0, 80, 320, 30);

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

static void setup_cleanup(WebServer& server, DNSServer& dnsServer) {
    server.stop();
    dnsServer.stop();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
}

static void show_setup_complete(void) {
    clear_screen();
    draw_title((char*)"Setup");
    draw_center_background(60, 120, 60);
    printTFTcentered("Setup complete", h.tft.color565(255, 255, 255), 2, 0, 80, 320, 30);
    delay(3000);
}

bool setup_process(Preferences& preferences) {
    char ap_ssid[32];
    char ap_pass[9];
    if (!start_ap(preferences, ap_ssid, ap_pass))
        return false;

    DNSServer dnsServer;
    WebServer server(80);
    SetupFormData form_data = {};

    display_ap_instructions(ap_ssid, ap_pass);
    run_captive_portal_until_form(server, dnsServer, form_data);

    connect_to_sta_wifi(form_data, preferences);

    int code = register_machine_to_api(form_data, preferences);
    if (code < 200 || code >= 300) {
        show_error_and_restart("API error");
        return false;
    }

    preferences.putString(MACHINE_NAME_KEY, form_data.machine_name);
    preferences.putString(MACHINE_API_HOST_KEY, form_data.api_host);
    preferences.putBool(SETUP_COMPLETED_KEY, true);

    setup_cleanup(server, dnsServer);
    show_setup_complete();
    return true;
}
