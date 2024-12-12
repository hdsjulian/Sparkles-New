#include "messageHandler.h"
#include "Arduino.h"
#include "esp_now.h"
#include "WiFi.h"

void MessageHandler::printAllPeers() {
       // Get the peer list
    esp_now_peer_info_t peerList;
    for (int i = 0;i<2;i++) {
        if (i == 0) {
            esp_now_fetch_peer(true, &peerList);
        }
        else {
            esp_now_fetch_peer(true, &peerList);
        }
        Serial.print("Peer ");
        Serial.print(": MAC Address=");
        for (int j = 0; j < 6; ++j) {
            Serial.print(peerList.peer_addr[j], HEX);
            if (j < 5) {
                Serial.print(":");
            }
        }
        Serial.print(", Channel=");
        Serial.print(peerList.channel);
        Serial.println();

    }
      
}

void MessageHandler::printAddress(const uint8_t * mac_addr){
    if (memcmp(mac_addr, hostAddress, 6) == 0) {
        Serial.println("HOST ADDRESS");
        return;
    }
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.println(macStr);
}

String MessageHandler::stringAddress(const uint8_t * mac_addr, bool debug){
    String macStr;
    if (memcmp(mac_addr, hostAddress, 6) == 0) {
        macStr += "HOST ADDRESS";
    }
    String separator = debug ? ", 0x" : ":";
    macStr += String(mac_addr[0], HEX);
    macStr += separator;
    macStr += String(mac_addr[1], HEX);
    macStr += separator; 
    macStr += String(mac_addr[2], HEX);
    macStr += separator; 
    macStr += String(mac_addr[3], HEX);
    macStr += separator; 
    macStr += String(mac_addr[4], HEX);
    macStr += separator; 
    macStr += String(mac_addr[5], HEX);

    return macStr;
}


int MessageHandler::addPeer(uint8_t * address) {
    memcpy(&peerInfo.peer_addr, address, 6);

    if (esp_now_get_peer(peerInfo.peer_addr, &peerInfo) == ESP_OK) {
        return 0;
    }
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        return -1;
    }
    else {
        return 1;
    }
    
}

void MessageHandler::removePeer(uint8_t address[6]) {
    if (esp_now_del_peer(address) != ESP_OK) {
        ESP_LOGI("peer", "coudln't delete peer");
    }
    return;
}

unsigned long long MessageHandler::timeDiffAbs(unsigned long long a, unsigned long long b) {
    return (a > b) ? (a - b) : (b - a);
}