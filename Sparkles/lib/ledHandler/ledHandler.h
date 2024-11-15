#include "../../include/myDefines.h"
#ifndef LED_HANDLER_H
#define LED_HANDLER_H
#include <queue>


class ledHandler {
    private:
    static void ledTaskWrapper(void *pvParameters);
    void ledTask();
    void ledsOff();
    void runMidi();
    void runStrobe();
    int timerOffset;
    int mode;
    int position;
    const float midiHue = 25/360;
    const float midiSat = 0.84;
    static void writeLeds(float rgb[3]);
    static float* hsv2rgb(float h, float s, float b, float* rgb);
    static float float_to_sRGB(float x);
    static float sRGB_to_float (float val);
    SemaphoreHandle_t configMutex;
    QueueHandle_t ledQueue;
    
    public:

    ledHandler();
    void setup();
    void startLedTask();
    void updatePosition();
    void updateTimerOffset();
    void addToMidiTable(midiNoteTable midiNoteTableArray[8], message_animate animation, int position);
    int getMidiNoteFromPosition(int position);
    float calcuateMidiDecay(unsigned long long startTime, int velocity, int note);
    int getDecayTime(int midiNote, int velocity);

};

#endif