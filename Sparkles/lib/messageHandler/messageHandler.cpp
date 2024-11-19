#include "messageHandler.h"
messageHandler* messageHandler::instance = nullptr;

messageHandler::messageHandler() {
    }

void messageHandler::setup(LedHandler &globalLedInstance) {
    ledInstance = &globalLedInstance;
    if (esp_now_init() != ESP_OK) {
        ESP_LOGI("ERROR", "Error initializing ESP-NOW");
        return;
    }
    esp_now_register_send_cb(onDataSent);
    messageQueue = xQueueCreate(10, sizeof(uint8_t *));
    xTaskCreatePinnedToCore(handleReceiveWrapper, "handleReceive", 10000, this, 1, NULL, 0);
    memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);
    
    
}
void messageHandler::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void messageHandler::onDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
    if (instance != nullptr) {
        instance->pushToRecvQueue(mac, incomingData, len);
    }
}
void messageHandler::pushToRecvQueue(const esp_now_recv_info *mac, const uint8_t *incomingData, int len) {
    xQueueSend(messageQueue, &incomingData, 0);
}

void messageHandler::handleReceiveWrapper(void *pvParameters) {
    messageHandler *messageHandlerInstance = (messageHandler *)pvParameters;
    messageHandlerInstance->handleReceive();
}

void messageHandler::handleReceive() {
    const uint8_t *incomingData;
    while (true) {
        if (xQueueReceive(messageQueue, &incomingData, portMAX_DELAY) == pdTRUE) {
            if (incomingData[0] == MSG_ANIMATION) {
                ESP_LOGI("MSG", "Received animation message");
                ESP_LOGI("MSG", "Animation type: %d", incomingData[1]);
                message_animate *animation = (message_animate *)incomingData;
                ESP_LOGI("MSG", "Midi note: %d", animation->animationParams.midi.note);
                ESP_LOGI("MSG", "Midi velocity: %d", animation->animationParams.midi.velocity);
                ledInstance->pushToAnimationQueue(*animation);
            }
        }
    }
}