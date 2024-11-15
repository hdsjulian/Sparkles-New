#include <ledHandler.h>

ledHandler::ledHandler() {
    configMutex = xSemaphoreCreateMutex();
}   

void ledHandler::setup() {
  ledcAttach(ledPinRed1, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttach(ledPinGreen1, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttach(ledPinBlue1, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttach(ledPinRed2, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttach(ledPinGreen2, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttach(ledPinBlue2, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);

  ledsOff();
}

void ledHandler::startLedTask() {
  xTaskCreatePinnedToCore(ledTaskWrapper, "ledTask", 10000, this, 1, NULL, 0);
}

void ledHandler::ledTaskWrapper(void *pvParameters) {
  ledHandler *ledHandlerInstance = (ledHandler *) pvParameters;
  ledHandlerInstance->ledTask();
}

void ledHandler::ledTask() {
    animationEnum animationType = OFF;
    unsigned long long animationStart = 0;
    message_animate animationData;
    midiNoteTable midiNoteTableArray[8];

    while (true) {
        if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
            int currentOffset = timerOffset;
            int currentMode = mode;
            int currentPosition = position;
            xSemaphoreGive(configMutex);
        }
        if (xQueueReceive(ledQueue, &animation, 0) == pdTRUE) {
            if (animation.animationType == MIDI) {
                addToMidiTable(midiNoteTableArray, animation);
            }
            else {
                animationData = animation;
            }
        }
        if (animationData.animationType == OFF) {
            ledsOff();
            continue;
        } else if (animationData.animationType == MIDI) {
            runMidi();
            continue;
        }
           
}

void ledHandler::addToMidiTable(midiNoteTable midiNoteTableArray[8], message_animate animation, int position) {
    if (animation.animationParams.midi.note % getMidiNoteFromPosition(position)+OCTAVE != 0) {
        return;
    }

    int note = animation.animationParams.midi.note;
    int velocity = animation.animationParams.midi.velocity;
    int octave = (note/OCTAVE)-1;
    if (midiNoteTableArray[octave].velocity < velocity) {
        midiNoteTableArray[octave].velocity = velocity;
        midiNoteTableArray[octave].note = note;
        midiNoteTableArray[octave].startTime = midiParams->velocity == 0 ? 0: micros();
    }
}


void ledHandler::runMidi(midiNoteTable midiNoteTableArray[8], int position) {
    int brightness = 0;
    float huemod = 0;
    float satmod = 0;
    for (int i = 0; i < 8; i++ ) {
        float midiDecayFactor = calculateMidiDecay(midiNoteTableArray[i].startTime, midiNoteTableArray[i].velocity, midiNoteTableArray[i].note);
        if (midiDecayFactor == 1) {
            midiNoteTableArray[i].velocity = 0;
            midiNoteTableArray[i].note = 0;
            midiNoteTableArray[i].startTime = 0;
        }
        else {
            int note = midiNoteTableArray[i].note;
            int velocity = midiNoteTableArray[i].velocity;
            int octave = (note/OCTAVE)-1;
            int ledOctave = getOctaveFromPosition(position);
            int octaveDistance = abs(octave-ledOctave);
            float distanceFactor = 0.2*octaveDistance;
            int currentBrightness = (int)(velocity*midiDecayFactor*(1-distanceFactor));
            if (currentBrightness > brightness) {
                brightness = currentBrightness;
                huemod = -0.02*octaveDistance;
                satmod = 0.02*octaveDistance;
            }
        }
    }
    float rgb[3];
    hsv2rgb(midiHue+huemod, midiSat+satmod, brightness/127, rgb);
    rgb[0] = float_to_sRGB(rgb[0])*255;
    rgb[1] = float_to_sRGB(rgb[1])*255;
    rgb[2] = float_to_sRGB(rgb[2])*255;
    writeLeds(rgb);
}

float ledHandler::calculateMidiDecay(unsigned long long startTime, int velocity, int note) {
    if (startTime == 0) {
        return 0;
    }
    unsigned long long currentTime = micros();
    unsigned long long timeElapsed = currentTime - startTime;
    int decay = getDecayTime(note, velocity);
    if (timeElapsed == 0) {
        return 1;
    }
    else if (timeElapsed > decay) {
        return 1;
    }
    else {
        return timeElapsed/decay;
        
    }
    
}

int ledHandler::getDecayTime(int midiNote, int velocity) {
    int minNote = 21;         // Lowest note on an 88-key keyboard (A0)
    int maxNote = 108;        // Highest note on an 88-key keyboard (C8)
    float minDecay = 8.0;     // Decay time for the lowest note in seconds
    float maxDecay = 2.0;     // Decay time for the highest note in seconds

    // Linear interpolation formula
    float decayTime = (minDecay - ((midiNote - minNote) * (minDecay - maxDecay) / (maxNote - minNote))) * velocity/127;
    return (int)decayTime*1000000;
}

int ledHandler::getMidiNoteFromPosition(int position) {
    return position+21;
}

int ledHandler::getOctaveFromPosition(int position) {
    return (getMidiNoteFromPosition(position)/OCTAVE)-1;
}

float* ledHandler::hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

float ledHandler::sRGB_to_float (float val)
{
    if( val < 0.04045 )
        val /= 12.92;
    else
        val = pow((val + 0.055)/1.055,2.4);
    return val;
}

float ledHandler::float_to_sRGB ( float val )
{
    if( val < 0.0031308 )
        val *= 12.92;
    else
        val = 1.055 * pow(val,1.0/2.4) - 0.055;
    return val;
}