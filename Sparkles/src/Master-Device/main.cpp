#include <Arduino.h>
#include <myDefines.h>
#include <esp_log.h>
#include "esp_now.h"
#include <LittleFS.h>
#include "WiFi.h"
#include <ledHandler.h>
#include <messageHandler.h>

// put function declarations here:

LedHandler ledInstance;
messageHandler handleMessages;

uint8_t myAddress[6];

// state
bool lfs_started = true;

void OnDataRecv(const esp_now_recv_info *mac, const uint8_t *incomingData, int len) {}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus) {}
unsigned long lastTick = 0;
void setup()
{
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_INFO);
  unsigned long long startTime = millis();
  while (!Serial)
  {
    if (millis() - startTime > 3000)
    {
      break;
    }
  }
  if (!LittleFS.begin())
  {
    Serial.println("LittleFS mount failed");
    lfs_started = false;
  }
  WiFi.mode(WIFI_AP_STA);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  ledInstance.setup();
  //handleMessages.setup(ledInstance);


  // put your setup code here, to run once:
}

void loop()
{
  if (lastTick + 1000 < millis())
  {
    lastTick = millis();
    ESP_LOGI("", "Tick");
  }
  // put your main code here, to run repeatedly:
}
