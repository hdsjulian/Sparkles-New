#include "Arduino.h"
#if DEVICE_USED == 4
#include "espnow.h"
#else 
#include "esp_now.h"
#endif

#include "time.h"
#ifndef DEFINE_H
#define DEFINE_H

//LEDHANDLER
#define LEDC_TIMER_12_BIT  8
#define LEDC_BASE_FREQ     5000
#define LEDC_START_DUTY   (0)
#define LEDC_TARGET_DUTY  (4095)
#define LEDC_FADE_TIME    (3000)

//device modes

#define MAIN 0
#define CLIENT 1
#define WEBSERVER 2

#define V1 1
#define V2 2
#define V3 3
#define V4 5

//magic numbers
#define OCTAVE 12


#if (DEVICE_USED == V1)
    const int ledPinBlue1 = 20;  // 16 corresponds to GPIO16
    const int ledPinRed1 = 9; // 17 corresponds to GPIO17
    const int ledPinGreen1 = 3;  // 5 corresponds to GPIO5
    const int ledPinGreen2 = 8;
    const int ledPinRed2 = 19;
    const int ledPinBlue2 = 18;

#elif (DEVICE_USED == V2)


    const int ledPinBlue1 = 18;  // 16 corresponds to GPIO16
    const int ledPinRed1 = 38; // 17 cmsgrorresponds to GPIO17
    const int ledPinGreen1 = 8;  // 5 corresponds to GPIO5
    const int ledPinGreen2 = 3;
    const int ledPinRed2 = 9;
    const int ledPinBlue2 = 37;
#elif (DEVICE_USED == V3)
    const int ledPinBlue1 = 17;  // 16 corresponds to GPIO16
    const int ledPinRed1 = 38; // 17 cmsgrorresponds to GPIO17
    const int ledPinGreen1 = 8;  // 5 corresponds to GPIO5
    const int ledPinGreen2 = 3;
    const int ledPinRed2 = 9;
    const int ledPinBlue2 = 37;
#elif (DEVICE_USED == V4)
    const int ledPinBlue1 = 17;  // 16 corresponds to GPIO16
    const int ledPinRed1 = 38; // 17 cmsgrorresponds to GPIO17
    const int ledPinGreen1 = 8;  // 5 corresponds to GPIO5
    const int ledPinGreen2 = 3;
    const int ledPinRed2 = 9;
    const int ledPinBlue2 = 37;
#endif



#define NUM_DEVICES 180

#define MSG_ANIMATION 5


enum animationEnum {
    OFF,
    FLASH,
    BLINK,
    CANDLE,
    SYNC_ASYNC_BLINK,
    SYNC_BLINK,
    SLOW_STARTUP,
    SYNC_END,
    LED_ON,
    CONCENTRIC, 
    STROBE, 
    MIDI
};
struct animation_strobe {
  uint8_t frequency;
  unsigned long long startTime;
  uint8_t duration;
  uint8_t rgb1[3];
  uint8_t rgb2[3];
  uint8_t brightness;
};

struct animation_midi {
  uint8_t note;
  uint8_t velocity;
  uint8_t octaveDistance;
};

union animation_params {
  struct animation_strobe strobe;
  struct animation_midi midi;
};


struct message_animate {
  uint8_t messageType = MSG_ANIMATION;
  animationEnum animationType;
  animation_params animationParams;
};


struct midiNoteTable {
  int velocity;
  int note;
  unsigned long long startTime;
  bool sustainPressed;
};
        

#endif