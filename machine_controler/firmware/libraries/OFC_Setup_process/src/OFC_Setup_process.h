#pragma once
#include <Preferences.h>
#include <OFC_Ui.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

class OFC_Setup_process {
    // Constructor / Destructor
    public:
        OFC_Setup_process(OFC_Ui* ui, Preferences* pref);
       ~OFC_Setup_process();

    // Public Methodes
    public:
        bool begin();

    // Private Methodes
    private:
        void generate_ap_credentials(char* ssid, size_t ssid_max, char* pass, size_t pass_len);
        void generate_uuid(char* out, size_t cap);
        bool start_ap(char* ap_ssid, char* ap_pass);
        void run_captive_portal_until_form();
        void connect_to_sta_wifi();
        int  register_machine_to_api();
        void setup_cleanup();

    // Private Attribut
    private:
        // External refs
        OFC_Ui*       _ui;
        Preferences*  _pref;

        // other attributs
        DNSServer     _dnsServer;
        WebServer     _server;
        SetupFormData _form_data = {};



}
