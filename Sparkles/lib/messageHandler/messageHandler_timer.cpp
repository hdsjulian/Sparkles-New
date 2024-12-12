#if DEVICE_MODE == MASTER
#include "messageHandler.h"
#include "Arduino.h"
#include "esp_now.h"
#include "WiFi.h"


void MessageHandler::startTimerSyncTask() {
    xTaskCreatePinnedToCore(runTimerSyncWrapper, "runTimerSync", 10000, this, 2, NULL, 0);
}
void MessageHandler::runTimerSyncWrapper(void *pvParameters) {
    MessageHandler *messageHandlerInstance = (MessageHandler *)pvParameters;
    messageHandlerInstance->runTimerSync();
}

void MessageHandler::runTimerSync() {
    message_timer timerMessage;
    message_data messageData;
    messageData.messageType = MSG_TIMER;
    setSettingTimer(true);
    int timerIndex  = getCurrentTimerIndex();
    if (timerIndex > -1) {
        addPeer(addressList[timerIndex].address);
    }
    while (getTimerSet() == false) {

        timerMessage.counter++;
        timerMessage.sendTime = micros();
        timerMessage.lastDelay = getLastDelay();
        timerMessage.reset = false;
        timerMessage.addressId = getCurrentTimerIndex();
        memcpy(&messageData.payload.timer, &timerMessage, sizeof(timerMessage));
        setLastSendTime(timerMessage.sendTime);
        if (timerIndex == -1) {
            esp_now_send(broadcastAddress, (uint8_t *) &messageData, sizeof(messageData));
        }
        else if (timerIndex > -1) {
            esp_now_send(addressList[timerIndex].address, (uint8_t *) &messageData, sizeof(messageData));
        }
        vTaskDelay(TIMER_FREQUENCY/portTICK_PERIOD_MS);
    }
}



#endif