#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "../OFC_Network.h"

Api::Api(String resource_uuid, String host) : resource_uuid(resource_uuid), host(host) {
    client.setInsecure();
}

Api::~Api() {
    http.end();
}