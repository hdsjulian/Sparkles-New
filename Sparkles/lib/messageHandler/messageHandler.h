#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H
#include <Arduino.h>
#include <esp_now.h>
#include <ledHandler.h>

class MessageHandler
{
public:
    static MessageHandler& getInstance() {
        static MessageHandler instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }

    void setup(LedHandler &globalHandleLed);
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void onDataRecv(const esp_now_recv_info *mac, const uint8_t *incomingData, int len);
    static void handleReceiveWrapper(void *pvParameters);
    void handleReceive();
    void pushToRecvQueue(const esp_now_recv_info *mac, const uint8_t *incomingData, int len);
    void pushToSendQueue(const uint8_t *mac_addr, esp_now_send_status_t status);


private:
    MessageHandler();
    // Delete copy constructor and assignment operator to prevent copying
    MessageHandler(const MessageHandler&) = delete;
    MessageHandler& operator=(const MessageHandler&) = delete;
    const uint8_t *incomingData;
    static constexpr uint8_t broadcastAddress[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    QueueHandle_t receiveQueue, sendQueue;
    esp_now_peer_info_t peerInfo;
    esp_now_peer_num_t peerNum;
    static MessageHandler* instance;
    LedHandler* ledInstance = nullptr;
};
#endif