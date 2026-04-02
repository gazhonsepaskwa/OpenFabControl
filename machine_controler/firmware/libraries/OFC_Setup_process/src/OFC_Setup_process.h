#pragma once
#include <Preferences.h>
#include <OFC_Ui.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

class OFC_Setup_process {
    // Constructor / Destructor
    public:
        OFC_Setup_process();
       ~OFC_Setup_process() = default;

    // Public Methodes
    public:
        bool begin(Preferences& preferences, OFC_Ui& ui);

    // Private Methodes
    private:
        void generate_ap_credentials(char* ssid, size_t ssid_max, char* pass, size_t pass_len);
        void generate_uuid(char* out, size_t cap);
        bool start_ap(char* ap_ssid, char* ap_pass);
        void run_captive_portal_until_form();
        void connect_to_sta_wifi();
        int  register_machine_to_api();
        void setup_cleanup();
        void display_ap_instructions(const char* ap_ssid, const char* ap_pass);
        void show_error_and_restart(const char* msg);
        void show_setup_complete();

    // Private Attribut
    private:
        // External refs
        OFC_Ui*       _ui;
        Preferences*  _pref;

        // other attributs
        DNSServer     _dnsServer;
        WebServer     _server;
        SetupFormData _form_data = {};
};
