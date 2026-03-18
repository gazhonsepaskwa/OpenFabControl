 #include "OFC_Network.h"

OFC_Network::OFC_Network()
    : SSID(""), password(""), last_wifi_check_ms(0), last_wifi_reconnect_ms(0),
      api(nullptr) {
}

OFC_Network::OFC_Network(String ssid, String password, String resource_uuid, String host)
    : SSID(ssid), password(password), last_wifi_check_ms(0), last_wifi_reconnect_ms(0) {
    api = new Api(resource_uuid, host);
}

OFC_Network::~OFC_Network() {
    if (api) {
        delete api;
        api = nullptr;
    }
}
