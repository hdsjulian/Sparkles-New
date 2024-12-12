#include <LedHandler.h>

LedHandler::LedHandler()
{
    configMutex = xSemaphoreCreateMutex();
    if (configMutex == NULL) {
        ESP_LOGI("ERROR", "Failed to create configMutex");
    }
    ledQueue = xQueueCreate(10, sizeof(message_animation)); // Ensure the queue is created
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
    //ledcAttach(LEDPINBLUE2, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
    
    ledsOff();
    ESP_LOGI("LED", "LED SETUP ENDED");
    startLedTask();
    ESP_LOGI("LED", "LED TASK STARTED");
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
    ESP_LOGI("LED", "Starting ledTask");
    xTaskCreatePinnedToCore(ledTaskWrapper, "ledTask", 10000, this, 2, NULL, 0);
}

void LedHandler::ledTaskWrapper(void *pvParameters)
{
    ESP_LOGI("LED", "Starting ledTaskWraphab ich beper");

    LedHandler *LedHandlerInstance = (LedHandler *)pvParameters;
    ESP_LOGI("LED", "Starting ledTaskWrapper2");
    LedHandlerInstance->ledTask();
}
void LedHandler::runMidiWrapper(void *pvParameters) {
    LedHandler *LedHandlerInstance = (LedHandler *)pvParameters;
    LedHandlerInstance->runMidi();
}
void LedHandler::runBlinkWrapper(void *pvParameters) {
    LedHandler *LedHandlerInstance = (LedHandler *)pvParameters;
    ESP_LOGI("LED", "Starting blink task");
    LedHandlerInstance->runBlink();
}

void LedHandler::runStrobeWrapper(void *pvParameters) {
    LedHandler *LedHandlerInstance = (LedHandler *)pvParameters;
    LedHandlerInstance->runStrobe();
}


void LedHandler::ledTask()
{
    animationEnum animationType = OFF;
    unsigned long long animationStart = 0;
    message_animation animationData;
    int currentOffset;
    int currentMode;
    int currentPosition;
    ESP_LOGI("LED", "Starting ledTask2");
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
                int timeSpent = micros()-animation.timeStamp;
                handleQueue(animation, animationData, currentPosition);
                
            }
            continue;
        }
        else {
            if (xQueueReceive(ledQueue, &animation, 0) == pdTRUE) 
            {   
                if (getCurrentAnimation() != MIDI) {
                    if (animationTaskHandle != NULL) {
                        vTaskDelete(animationTaskHandle);
                        animationTaskHandle = NULL;
                    }
                }
                if (animation.animationType == MIDI && getCurrentAnimation() != MIDI) {
                    if (midiTaskHandle != NULL) {
                        vTaskDelete(midiTaskHandle);
                        midiTaskHandle = NULL;
                    }
                }
                handleQueue(animation, animationData, currentPosition);
            }
        }    
         if (getCurrentAnimation() == MIDI){   
            if (midiTaskHandle == NULL || eTaskGetState(midiTaskHandle) == eDeleted) {
                xTaskCreate(runMidiWrapper, "runMidi", 10000, this, 1, &midiTaskHandle); // Create the runMidi task
            }
        }           
        else if (getCurrentAnimation() == STROBE)
            {   
                ESP_LOGI("LED", "Received Strobe");
                if (animationTaskHandle == NULL || eTaskGetState(animationTaskHandle) == eDeleted) {
                    xTaskCreate(runStrobeWrapper, "runStrobe", 10000, this, 1, &animationTaskHandle);
                }
            }
        else if (getCurrentAnimation() == BLINK)
            {   
                if (animationTaskHandle == NULL || eTaskGetState(animationTaskHandle) == eDeleted) {
                    ESP_LOGI("LED", "Starting blink task");
                    xTaskCreate(runBlinkWrapper, "runBlink", 10000, this, 1, &animationTaskHandle);
                }
            }
        else
            {
                //ESP_LOGI("LED", "Unknown animation type %d", animationData.animationType);
            }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}


void LedHandler::handleQueue(message_animation& animation, message_animation& animationData, int currentPosition) {
    if (animation.animationType == MIDI) {
        addToMidiTable(midiNoteTableArray, animation, position);
    }
    else if (animation.animationType == OFF) {
        ledsOff();
        setCurrentAnimation(OFF);
    }
    animationData = animation;
    setCurrentAnimation(animation.animationType);

}

void LedHandler::runStrobe() {
    unsigned long long microsUntilStart = animation.animationParams.strobe.startTime-micros();
    TickType_t ticksUntilStart = microsToTicks(microsUntilStart);
    TickType_t currentTicks = xTaskGetTickCount();
    unsigned long long endTimeMicros = animation.animationParams.strobe.duration+animation.animationParams.strobe.startTime;
    TickType_t endTimeTicks = microsToTicks(endTimeMicros);

    vTaskDelayUntil(&currentTicks, ticksUntilStart);
    ledsOff();
    float rgb[3];
    ESP_LOGI("LED", "Starting in %d", microsUntilStart);
    while (true) {
        hsv2rgb(animation.animationParams.strobe.hue, animation.animationParams.strobe.saturation, animation.animationParams.strobe.brightness, rgb);
        writeLeds(rgb);
        vTaskDelay(1000/animation.animationParams.strobe.frequency);
        if (xTaskGetTickCount() >= endTimeTicks) {
            ledsOff();
            setCurrentAnimation(OFF);
            animationTaskHandle = NULL;
            vTaskDelete(NULL);
        }
        else {
            ledsOff();
            vTaskDelay(1000/animation.animationParams.strobe.frequency);
        }
    }
    
}

void LedHandler::runBlink() {
    if (animation.animationParams.blink.startTime > micros()) {
        unsigned long long microsUntilStart = animation.animationParams.blink.startTime-micros();
        TickType_t ticksUntilStart = microsToTicks(microsUntilStart);
        TickType_t currentTicks = xTaskGetTickCount();
        vTaskDelayUntil(&currentTicks, ticksUntilStart);
    }
    float rgb[3];
    for (int i = 0; i < animation.animationParams.blink.repetitions; i++) {
        hsv2rgb(animation.animationParams.blink.hue, animation.animationParams.blink.saturation, animation.animationParams.blink.brightness, rgb);
        writeLeds(rgb);
        vTaskDelay(animation.animationParams.blink.duration);
        ledsOff();
        vTaskDelay(animation.animationParams.blink.duration);
    }
    setCurrentAnimation(OFF);
    animationTaskHandle = NULL;
    vTaskDelete(NULL);
    
}
void LedHandler::pushToAnimationQueue(message_animation animation)
{   
    //ESP_LOGI("MSG", "ONE");
    xQueueSend(ledQueue, &animation, 0);
}



void placeholder(int octave, int octaveDistance, float distanceFactor, float currentBrightness, float midiDecayFactor, float note) {
    ESP_LOGI("PLACEHOLDER", "octave: %d, octaveDistance: %d, distanceFactor: %.2f, currentBrightness: %.2f, midiDecayFactor: %.2f, note: %.2f", octave, octaveDistance, distanceFactor, currentBrightness, midiDecayFactor, note);
    return;
}


void LedHandler::runMidi()
{   
    float brightness = 0.0;
    float huemod = 0;
    float satmod = 0;
    midiNoteTable localMidiNoteTableArray[OCTAVESONKEYBOARD];
    while (true) {
        getMidiNoteTableArray(localMidiNoteTableArray, sizeof(localMidiNoteTableArray));
        bool brightnessZero = true;
        bool newBrightness = false;
        brightness = 0.0;
        for (int i = 0; i < OCTAVESONKEYBOARD; i++)
        {   
            if (localMidiNoteTableArray[i].velocity == 0)
            {
                continue;
            }
            float midiDecayFactor = calculateMidiDecay(localMidiNoteTableArray[i].startTime, localMidiNoteTableArray[i].velocity, localMidiNoteTableArray[i].note);
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
            vTaskDelete(NULL);
        }
        else {
            float rgb[3];
            hsv2rgb(max(midiHue + huemod, 0.0f), midiSat + satmod, brightness / 127, rgb);

            if (newBrightness == true)
            {   
                ESP_LOGI("LED", "Hue: %.2f, Saturation: %.2f, Brightness: %.2f", midiHue, midiSat, brightness / 127);
                ESP_LOGI("LED", "Brightness: %.2f, rgb: %d, %d, %d", brightness, (int)rgb[0], (int)rgb[1], (int)rgb[2]);
            }
            writeLeds(rgb);
        }
        vTaskDelay((1000/FPS)/portTICK_PERIOD_MS);
    }
}


void LedHandler::runBlinkOld() {
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


