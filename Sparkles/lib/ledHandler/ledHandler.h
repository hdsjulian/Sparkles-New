#include "../../include/myDefines.h"
#ifndef LED_HANDLER_H
#define LED_HANDLER_H
#include <queue>

#define FPS 60
class LedHandler
{
public:
    static LedHandler& getInstance() {
        static LedHandler instance; // Guaranteed to be destroyed and instantiated on first use
        return instance;
    }

    void setup();
    void startLedTask();
    void updatePosition();
    void updateTimerOffset();
    void addToMidiTable(midiNoteTable midiNoteTableArray[OCTAVESONKEYBOARD], message_animate animation, int position);
    void pushToAnimationQueue(message_animate animation);
    static void runBlink();
    void setTimerOffset(int newOffset);
    int getTimerOffset();
    void setMidiNoteTable(int index, midiNoteTable note);
    midiNoteTable getMidiNoteTable(int index);
    void getMidiNoteTableArray(midiNoteTable* buffer, size_t size);
    animationEnum getCurrentAnimation();
    void setCurrentAnimation(animationEnum animation);

private:
    LedHandler();
    LedHandler(const LedHandler&) = delete;
    LedHandler& operator=(const LedHandler&) = delete;
    static void ledTaskWrapper(void *pvParameters);
    static void runMidiWrapper(void *pvParameters);
    void ledTask();
    void runMidi();
    static void ledsOff();
    static void runStrobe();
    int timerOffset;
    int mode;
    int position;
    static constexpr float midiHue = 25 / 360;
    static constexpr float midiSat = 0.84;
    static void writeLeds(float rgb[3]);
    static float *hsv2rgb(float h, float s, float b, float *rgb);
    static float float_to_sRGB(float x);
    static float sRGB_to_float(float val);
    static float fract(float x);
    static float mix(float a, float b, float t);
    static float step(float edge, float x);
    static int getOctaveFromPosition(int position);
    static float calculateMidiDecay(unsigned long long startTime, int velocity, int note);
    static int getDecayTime(int midiNote, int velocity);
    static int getMidiNoteFromPosition(int position);
    void handleQueue(message_animate& animation, message_animate& animationData, int currentPosition);

    SemaphoreHandle_t configMutex;
    QueueHandle_t ledQueue;
    message_animate animation;
    midiNoteTable midiNoteTableArray[OCTAVESONKEYBOARD];
    SemaphoreHandle_t midiNoteTableMutex; 
    animationEnum currentAnimation;
    TaskHandle_t midiTaskHandle;
};

#endif