#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "../OFC_Network.h"

Api::Api(String resource_uuid, String host) : _resource_uuid(resource_uuid), _host(host) {
    _client.setInsecure();
}

Api::~Api() {
    _http.end();
}
