#include "messageHandler.h"

#if DEVICE_MODE == MASTER
#include "Arduino.h"
#include "esp_now.h"
#include "WiFi.h"

void MessageHandler::handleReceive() {
    message_data incomingData;
    
    while (true) {

        if (xQueueReceive(receiveQueue, &incomingData, portMAX_DELAY) == pdTRUE) {
            //BETA
            //ESP_LOGI("MSG", "Received from queue");
            if (incomingData.messageType == MSG_ADDRESS) {
                vTaskDelay(1000/portTICK_PERIOD_MS);
                //ESP_LOGI("MSG", "Received animation message");
                //ESP_LOGI("MSG", "Animation type: %d", incomingData[1]);
                //message_animate *animation = (message_animate *)incomingData;
                //ESP_LOGI("MSG", "Midi note: %d", animation->animationParams.midi.note);
                //ESP_LOGI("MSG", "Midi velocity: %d", animation->animationParams.midi.velocity);
                message_address address = (message_address)incomingData.payload.address;
                int index = addOrGetAddressId(address.address);
                setCurrentTimerIndex(index);
                if (timerSyncHandle == NULL) {
                    xTaskCreatePinnedToCore(runTimerSyncWrapper, "runTimerSync", 2000, this, 3, &timerSyncHandle, 0);
                }
            }
            else if (incomingData.messageType == MSG_GOT_TIMER) {
                vTaskDelete(timerSyncHandle);
                removePeer(addressList[getCurrentTimerIndex()].address);
                timerSyncHandle = NULL;
                setCurrentTimerIndex(-1);

            }
            else {
                ESP_LOGI("MSG", "Unknown message type ");

            }
        }
    }
}

void MessageHandler::handleSend() {
    message_data messageData;

    while(true) {
        if (xQueueReceive(sendQueue, &messageData, portMAX_DELAY) == pdTRUE) {
            switch (messageData.messageType) {
                case MSG_ADDRESS:
                    esp_now_send(messageData.address, (uint8_t *) &messageData, sizeof(messageData));
                    break;
                default:
                    esp_now_send(messageData.address, (uint8_t *) &messageData, sizeof(messageData));
                    break;
            }        
        }
    }
}

void MessageHandler::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    MessageHandler& instance = getInstance();
    if (status == ESP_NOW_SEND_SUCCESS) {
        if (instance.getSettingTimer() == true) {
            if (memcmp(mac_addr, instance.getItemFromAddressList(instance.getCurrentTimerIndex()).address, 6) == 0) {
                instance.setLastDelay(micros() - instance.getLastSendTime());   
            }
            else {
                ESP_LOGI("MSG", "Message sent to %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
            }
        }
    }
    else {
        ESP_LOGI("MSG", "Send failed");
    }
}

void MessageHandler::onDataRecv(const esp_now_recv_info * mac, const uint8_t *incomingData, int len) {
    //ALPHA

    MessageHandler& instance = getInstance();
    instance.pushToRecvQueue(mac, incomingData, len);
}

#endif