#include "messageHandler.h"
#include "Arduino.h"
#include "esp_now.h"
#include "WiFi.h"

int MessageHandler::getCurrentTimerIndex() {
    int returnIndex ; 
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnIndex = currentTimerIndex;
        xSemaphoreGive(configMutex);
    }
    return returnIndex;
}
void MessageHandler::setCurrentTimerIndex(int index) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        currentTimerIndex = index;
        xSemaphoreGive(configMutex);
    }
}
void MessageHandler::setTimerSet(bool set) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        timerSet = set;
        xSemaphoreGive(configMutex);
    }
}
bool MessageHandler::getTimerSet() {
    bool returnSet;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnSet = timerSet;
        xSemaphoreGive(configMutex);
    }
    return returnSet;
}
void MessageHandler::setSettingTimer(bool set) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        settingTimer = set;
        xSemaphoreGive(configMutex);
    }
}
bool MessageHandler::getSettingTimer() {
    bool returnSettingTimer;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnSettingTimer = settingTimer;
        xSemaphoreGive(configMutex);
    }
    return returnSettingTimer;
}

void MessageHandler::setLastSendTime(int time) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        lastSendTime = time;
        xSemaphoreGive(configMutex);
    }
}
int MessageHandler::getLastSendTime() {
    int returnTime;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnTime = lastSendTime;
        xSemaphoreGive(configMutex);
    }
    return returnTime;
}
void MessageHandler::setLastDelay(int delay) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        lastDelay = delay;
        xSemaphoreGive(configMutex);
    }
}
int MessageHandler::getLastDelay() {
    int returnDelay;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnDelay = lastDelay;
        xSemaphoreGive(configMutex);
    }
    return returnDelay;
}

client_address MessageHandler::getItemFromAddressList(int index) {
    client_address returnItem;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnItem = addressList[index];
        xSemaphoreGive(configMutex);
    }
    return returnItem;
}
void MessageHandler::setItemFromAddressList(int index, client_address item) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        addressList[index] = item;
        xSemaphoreGive(configMutex);
    }
}
void MessageHandler::setMsgReceiveTime(int time) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        msgReceiveTime = time;
        xSemaphoreGive(configMutex);
    }
}
unsigned long long MessageHandler::getMsgReceiveTime() {
    unsigned long long returnTime;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnTime = msgReceiveTime;
        xSemaphoreGive(configMutex);
    }
    return returnTime;
}

void MessageHandler::setAddressAnnounced(bool set) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        addressAnnounced = set;
        xSemaphoreGive(configMutex);
    }
}
bool MessageHandler::getAddressAnnounced() {
    bool returnSet;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnSet = addressAnnounced;
        xSemaphoreGive(configMutex);
    }
    return returnSet;
}
int MessageHandler::addOrGetAddressId(uint8_t * address) {
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (memcmp(addressList[i].address, address, 6) == 0) {
            return i;
        }
        if (memcmp(addressList[i].address, emptyAddress, 6) == 0) {
            memcpy(addressList[i].address, address, 6);
            return i;
        }
    }
    return -1;
}

void MessageHandler::setTimeOffset(unsigned long long sendTime, unsigned long long receiveTime, int delay) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        if (receiveTime < sendTime) {
            timeOffset = sendTime-receiveTime - delay/2;
            offsetMultiplier = -1;
        }
        else {
            timeOffset = receiveTime-sendTime - delay/2;
            offsetMultiplier = 1;
        }
        xSemaphoreGive(configMutex);

    }
}
 long long MessageHandler::getTimeOffset() {
     long long returnOffset;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnOffset = timeOffset*offsetMultiplier;
        xSemaphoreGive(configMutex);
    }
    return returnOffset;
}

void MessageHandler::setLastReceiveTime(unsigned long long time) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        lastReceiveTime = time;
        xSemaphoreGive(configMutex);
    }
}
unsigned long long MessageHandler::getLastReceiveTime() {
    int returnTime;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        returnTime = lastReceiveTime;
        xSemaphoreGive(configMutex);
    }
    return returnTime;
}