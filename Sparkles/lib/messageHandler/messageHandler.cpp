#include "messageHandler.h"
MessageHandler* MessageHandler::instance = nullptr;

MessageHandler::MessageHandler() {
    receiveQueue = xQueueCreate(10, sizeof(message_animate));
    if (receiveQueue == NULL) {
        ESP_LOGI("ERROR", "Failed to create receiveQueue");
    }
    sendQueue = xQueueCreate(10, sizeof(message_animate));
    if (sendQueue == NULL) {
        ESP_LOGI("ERROR", "Failed to create sendQueue");
    }
}

void MessageHandler::setup(LedHandler &globalLedInstance) {
    ledInstance = &globalLedInstance;
    if (esp_now_init() != ESP_OK) {
        ESP_LOGI("ERROR", "Error initializing ESP-NOW");
        return;
    }
    xTaskCreatePinnedToCore(handleReceiveWrapper, "handleReceive", 10000, this, 2, NULL, 0);
    memcpy(&peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_register_send_cb(onDataSent);
    esp_now_register_recv_cb(onDataRecv);
    
}
void MessageHandler::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
}

void MessageHandler::onDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
    //ESP_LOGI("Received", "Data at %d", micros());
    //ALPHA

    MessageHandler& instance = getInstance();
    instance.pushToRecvQueue(mac, incomingData, len);
}
void MessageHandler::pushToRecvQueue(const esp_now_recv_info *mac, const uint8_t *incomingData, int len) {
    message_animate msg;
    memcpy(&msg, incomingData, len);
    msg.timeStamp = micros();

    xQueueSend(receiveQueue, &msg, 0);
}


void MessageHandler::handleReceiveWrapper(void *pvParameters) {
    MessageHandler *messageHandlerInstance = (MessageHandler *)pvParameters;
    messageHandlerInstance->handleReceive();
}

void MessageHandler::handleReceive() {
    const uint8_t *incomingData;
    message_animate animation;
    while (true) {
        if (xQueueReceive(receiveQueue, &animation, portMAX_DELAY) == pdTRUE) {
            //BETA
            
            //ESP_LOGI("MSG", "Received from queue");
            if (animation.messageType == MSG_ANIMATION) {
                //ESP_LOGI("MSG", "Received animation message");
                //ESP_LOGI("MSG", "Animation type: %d", incomingData[1]);
                //message_animate *animation = (message_animate *)incomingData;
                //ESP_LOGI("MSG", "Midi note: %d", animation->animationParams.midi.note);
                //ESP_LOGI("MSG", "Midi velocity: %d", animation->animationParams.midi.velocity);
                
                ledInstance->pushToAnimationQueue(animation);
            }
            else {
                ESP_LOGI("MSG", "Unknown message type ");
                ESP_LOGI("MSG", "Received animation message");
                ESP_LOGI("MSG", "Animation type: %d", animation.animationType);
                ESP_LOGI("MSG", "Midi note: %d", animation.animationParams.midi.note);
                ESP_LOGI("MSG", "Midi velocity: %d", animation.animationParams.midi.velocity);

            }
        }
    }
}