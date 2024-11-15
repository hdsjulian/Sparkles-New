#include <Arduino.h>
#include <myDefines.h>
#include <esp_log.h>
#include <ledHandler.h>
// put function declarations here:


void setup() {
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_INFO); 
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
