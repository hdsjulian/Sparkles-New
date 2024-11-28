#include <LedHandler.h>

LedHandler::LedHandler()
{
    configMutex = xSemaphoreCreateMutex();
    if (configMutex == NULL) {
        ESP_LOGI("ERROR", "Failed to create configMutex");
    }
    ledQueue = xQueueCreate(10, sizeof(message_animate)); // Ensure the queue is created
    if (ledQueue == NULL) {
        Serial.println("Failed to create ledQueue");
    }
    midiNoteTableMutex = xSemaphoreCreateMutex();
    if (midiNoteTableMutex == NULL) {
        ESP_LOGI("ERROR", "Failed to create midiNoteTableMutex");
    }
}

void LedHandler::setup()
{

    ledcAttach(LEDPINRED1, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    
    ledcAttach(LEDPINBLUE1, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    
    ledcAttach(LEDPINGREEN1, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttach(LEDPINRED2, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttach(LEDPINGREEN2, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    ledcAttach(LEDPINBLUE2, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    
    ledsOff();
    startLedTask();
}

void LedHandler::ledsOff() {
    ledcWrite(LEDPINRED1, 0);
    ledcWrite(LEDPINGREEN1, 0);
    ledcWrite(LEDPINBLUE1, 0);
    ledcWrite(LEDPINRED2, 0);
    ledcWrite(LEDPINGREEN2, 0);
    ledcWrite(LEDPINBLUE2, 0);
  
}

void LedHandler:: writeLeds(float rgb[3]) {
    //ESP_LOGI("LED", "writing leds %d, %d, %d ", (int)rgb[0],(int)rgb[1], (int)rgb[2]);
    ledcWrite(LEDPINRED1, (int)rgb[0]);
    ledcWrite(LEDPINGREEN1, (int) rgb[1]);
    ledcWrite(LEDPINBLUE1, (int)rgb[2]);
    ledcWrite(LEDPINRED2,(int) rgb[0]);
    ledcWrite(LEDPINGREEN2, (int)rgb[1]);
    ledcWrite(LEDPINBLUE2, (int)rgb[2]);
}

void LedHandler::startLedTask()
{
    xTaskCreatePinnedToCore(ledTaskWrapper, "ledTask", 10000, this, 2, NULL, 0);
}

void LedHandler::ledTaskWrapper(void *pvParameters)
{
    LedHandler *LedHandlerInstance = (LedHandler *)pvParameters;
    LedHandlerInstance->ledTask();
}
void LedHandler::runMidiWrapper(void *pvParameters) {
    LedHandler *LedHandlerInstance = (LedHandler *)pvParameters;
    LedHandlerInstance->runMidi();
}

void LedHandler::ledTask()
{
    animationEnum animationType = OFF;
    unsigned long long animationStart = 0;
    message_animate animationData;
    int currentOffset;
    int currentMode;
    int currentPosition;

    while (true)
    {
        if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE)
        {
            int currentOffset = timerOffset;
            int currentMode = mode;
            int currentPosition = position;
            xSemaphoreGive(configMutex);
        }
        if (getCurrentAnimation() == OFF)
        {   
            ledsOff();
            if (xQueueReceive(ledQueue, &animation, portMAX_DELAY) == pdTRUE) 
            {
                ESP_LOGI("Received", "LED receive Data at %d", micros()-animation.timeStamp);
                int timeSpent = micros()-animation.timeStamp;
                handleQueue(animation, animationData, currentPosition);
                
            }
            ESP_LOGI("LED", "Current animation: %d", getCurrentAnimation());
            continue;
        }
        else {
            if (xQueueReceive(ledQueue, &animation, 0) == pdTRUE) 
            {   
                handleQueue(animation, animationData, currentPosition);
            }
        }    
         if (getCurrentAnimation() == MIDI){   
            if (midiTaskHandle == NULL || eTaskGetState(midiTaskHandle) == eDeleted) {
                ESP_LOGI("LED", "Creating MIDI task");
                xTaskCreate(runMidiWrapper, "runMidi", 10000, this, 1, &midiTaskHandle); // Create the runMidi task
            }
        }           
        if (getCurrentAnimation() == STROBE)
            {
                runStrobe();
            }
        else
            {
                //ESP_LOGI("LED", "Unknown animation type %d", animationData.animationType);
            }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}


void LedHandler::handleQueue(message_animate& animation, message_animate& animationData, int currentPosition) {
    if (animation.animationType == MIDI) {
        addToMidiTable(midiNoteTableArray, animation, position);
    }
    animationData = animation;
    setCurrentAnimation(animation.animationType);

}

void LedHandler::runStrobe() {
    
}

void LedHandler::pushToAnimationQueue(message_animate animation)
{   
    //ESP_LOGI("MSG", "ONE");
    xQueueSend(ledQueue, &animation, 0);
}

void LedHandler::addToMidiTable(midiNoteTable midiNoteTableArray[OCTAVESONKEYBOARD], message_animate animation, int position)
{
    if (animation.animationParams.midi.note % OCTAVE != getMidiNoteFromPosition(position) % OCTAVE)
    {
        return;
    }

    int note = animation.animationParams.midi.note;
    int velocity = animation.animationParams.midi.velocity;
    int octave = (note / OCTAVE) - 1;
    if (xSemaphoreTake(midiNoteTableMutex, portMAX_DELAY) == pdTRUE) {
        if (midiNoteTableArray[octave].velocity < velocity)
        {
            midiNoteTableArray[octave].velocity = velocity;
            midiNoteTableArray[octave].note = note;
            midiNoteTableArray[octave].startTime = velocity == 0 ? 0 : micros();
            //ESP_LOGI("LED", "Added: Note: %d, Velocity: %d, Octave: %d", note, velocity, octave);
            //ESP_LOGI("LED", "Start time: %llu", midiNoteTableArray[octave].startTime);
        }
        else if (velocity == 0)
        {
            midiNoteTableArray[octave].velocity = 0;
            midiNoteTableArray[octave].note = 0;
            midiNoteTableArray[octave].startTime = 0;
            //ESP_LOGI("LED", "Set to Zero");
        }
        xSemaphoreGive(midiNoteTableMutex);
    }
}

void placeholder(int octave, int octaveDistance, float distanceFactor, float currentBrightness, float midiDecayFactor, float note) {
    ESP_LOGI("PLACEHOLDER", "octave: %d, octaveDistance: %d, distanceFactor: %.2f, currentBrightness: %.2f, midiDecayFactor: %.2f, note: %.2f", octave, octaveDistance, distanceFactor, currentBrightness, midiDecayFactor, note);
    return;
}


void LedHandler::runMidi()
{   
    float brightness = 0;
    float huemod = 0;
    float satmod = 0;
    midiNoteTable localMidiNoteTableArray[OCTAVESONKEYBOARD];
    while (true) {
        getMidiNoteTableArray(localMidiNoteTableArray, sizeof(localMidiNoteTableArray));
        bool brightnessZero = true;
        for (int i = 0; i < OCTAVESONKEYBOARD; i++)
        {   
            if (localMidiNoteTableArray[i].velocity == 0)
            {
                continue;
            }
            float midiDecayFactor = calculateMidiDecay(localMidiNoteTableArray[i].startTime, localMidiNoteTableArray[i].velocity, localMidiNoteTableArray[i].note);
            ESP_LOGI("LED", "Decay factor: %f at %d", midiDecayFactor, micros());
            if (midiDecayFactor == 0.0)
            {
                continue;
            }
            //ESP_LOGI("LED", "Decay factor: %f at %d", midiDecayFactor, micros());
            if (midiDecayFactor == 1)
            {
                localMidiNoteTableArray[i].velocity = 0;
                localMidiNoteTableArray[i].note = 0;
                localMidiNoteTableArray[i].startTime = 0;
            }
            else
            {
                int note = localMidiNoteTableArray[i].note;
                int velocity = localMidiNoteTableArray[i].velocity;
                int octave = (note / OCTAVE) - 1;
                int ledOctave = getOctaveFromPosition(position);
                int octaveDistance = abs(octave - ledOctave);
                float distanceFactor = 0.2 * octaveDistance;
                float currentBrightness = (int)(velocity * (1-midiDecayFactor) * (1 - distanceFactor));
                if (currentBrightness > brightness)
                {
                    brightness = currentBrightness;
                    huemod = -0.02 * octaveDistance;
                    satmod = 0.02 * octaveDistance;
                }
                brightnessZero = false;

                            
            }
        }
        if (brightness == 0 || brightnessZero == true)  
        {   
            ledsOff();
            setCurrentAnimation(OFF);
            ESP_LOGI("LED", "Brightness is 0");
            vTaskDelete(NULL);
        }
        else {
            float rgb[3];
            hsv2rgb(midiHue + huemod, midiSat + satmod, brightness / 127, rgb);
            ESP_LOGI("LED", "Brightness: %.2f before: r: %.2f, g: %.2f, b: %.2f", brightness, rgb[0], rgb[1], rgb[2]);
            rgb[0] = float_to_sRGB(rgb[0]) * 255;
            rgb[1] = float_to_sRGB(rgb[1]) * 255;
            rgb[2] = float_to_sRGB(rgb[2]) * 255;
            writeLeds(rgb);
        }
        vTaskDelay((1000)/portTICK_PERIOD_MS);
    }
}
float LedHandler::calculateMidiDecay(unsigned long long startTime, int velocity, int note)
{
    if (startTime == 0)
    {
        return 0;
    }
    unsigned long long currentTime = micros();
    unsigned long long timeElapsed = currentTime - startTime;
    int decay = getDecayTime(note, velocity);
    //ESP_LOGI("LED", "Decay time: %d", decay);
    //ESP_LOGI("LED", "Time elapsed: %llu", timeElapsed);
    if (timeElapsed == 0)
    {
        return 1;
    }
    else if (timeElapsed > decay)
    {
        return 1;
    }
    else
    {
        float decayFactor = (float) timeElapsed / float(decay);
        return decayFactor;
    }
}

int LedHandler::getDecayTime(int midiNote, int velocity)
{
    int minNote = 21;     // Lowest note on an 88-key keyboard (A0)
    int maxNote = 108;    // Highest note on an 88-key keyboard (C8)
    float minDecay = 8.0; // Decay time for the lowest note in seconds
    float maxDecay = 2.0; // Decay time for the highest note in seconds

    // Linear interpolation formula
    float decayTime = (minDecay - ((midiNote - minNote) * (minDecay - maxDecay) / (maxNote - minNote))) * velocity / 127;
    return (int)decayTime * 1000000;
}

int LedHandler::getMidiNoteFromPosition(int position)
{
    return position + 21;
}

int LedHandler::getOctaveFromPosition(int position)
{
    return (getMidiNoteFromPosition(position) / OCTAVE) - 1;
}

float *LedHandler::hsv2rgb(float h, float s, float b, float *rgb)
{
    rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
    rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
    rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
    return rgb;
}

float LedHandler::sRGB_to_float(float val)
{
    if (val < 0.04045)
        val /= 12.92;
    else
        val = pow((val + 0.055) / 1.055, 2.4);
    return val;
}

float LedHandler::float_to_sRGB(float val)
{
    if (val < 0.0031308)
        val *= 12.92;
    else
        val = 1.055 * pow(val, 1.0 / 2.4) - 0.055;
    return val;
}

float LedHandler::fract(float x) { return x - int(x); }

float LedHandler::mix(float a, float b, float t) { return a + (b - a) * t; }

float LedHandler::step(float e, float x) { return x < e ? 0.0 : 1.0; }


void LedHandler::runBlink() {
    float targetRgb[3] = {50, 0, 0};
    float currentRgb[3] = {0, 0, 0};
    int steps = 100; // Number of steps for fading
    int delayTime = 10; // Delay time in milliseconds for each step

    // Increase brightness
    for (int i = 0; i <= steps; i++) {
        currentRgb[0] = (targetRgb[0] / steps) * i;
        currentRgb[1] = (targetRgb[1] / steps) * i;
        currentRgb[2] = (targetRgb[2] / steps) * i;
        //ESP_LOGI("LED", "Brightness: %.2f, rgb: %d, %d, %d", (float)i, (int)currentRgb[0], (int)currentRgb[1], (int)currentRgb[2]);
        writeLeds(currentRgb);
        vTaskDelay(pdMS_TO_TICKS(delayTime));
    }

    // Decrease brightness
    for (int i = steps; i >= 0; i--) {
        currentRgb[0] = (targetRgb[0] / steps) * i;
        currentRgb[1] = (targetRgb[1] / steps) * i;
        currentRgb[2] = (targetRgb[2] / steps) * i;
        writeLeds(currentRgb);
        vTaskDelay(pdMS_TO_TICKS(delayTime));
    }
}

void LedHandler::setTimerOffset(int newOffset) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        timerOffset = newOffset;
        xSemaphoreGive(configMutex);
    }
}

// Thread-safe getter for offset
int LedHandler::getTimerOffset() {
    int currentOffset;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        currentOffset = timerOffset;
        xSemaphoreGive(configMutex);
    }
    return currentOffset;
}


void LedHandler::setMidiNoteTable(int index, midiNoteTable tableElement) {
    if (xSemaphoreTake(midiNoteTableMutex, portMAX_DELAY) == pdTRUE) {
        midiNoteTableArray[index] = tableElement;
        xSemaphoreGive(midiNoteTableMutex);
    }
}

// Thread-safe getter for midiNoteTableArray
midiNoteTable LedHandler::getMidiNoteTable(int index) {
    midiNoteTable tableElement;
    if (xSemaphoreTake(midiNoteTableMutex, portMAX_DELAY) == pdTRUE) {
        tableElement = midiNoteTableArray[index];
        xSemaphoreGive(midiNoteTableMutex);
    }
    return tableElement;
}

animationEnum LedHandler::getCurrentAnimation() {
    animationEnum animation;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        currentAnimation = currentAnimation;
        xSemaphoreGive(configMutex);
    }
    ESP_LOGI("LED", "Getting current animation %d", animation);
    return animation;
}
void LedHandler::setCurrentAnimation(animationEnum animation) {
    ESP_LOGI("LED", "Setting current animation to %d", animation);
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        currentAnimation = animation;
        xSemaphoreGive(configMutex);
    }
}
void LedHandler::getMidiNoteTableArray(midiNoteTable* buffer, size_t size) {
    if (size < sizeof(midiNoteTableArray)) {
        // Handle error: buffer is too small
        return;
    }
    if (xSemaphoreTake(midiNoteTableMutex, portMAX_DELAY) == pdTRUE) {
        memcpy(buffer, midiNoteTableArray, sizeof(midiNoteTableArray));
        xSemaphoreGive(midiNoteTableMutex);
    }
}