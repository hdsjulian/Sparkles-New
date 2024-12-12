#include <LedHandler.h>
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


animationEnum LedHandler::getCurrentAnimation() {
    animationEnum animation;
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        animation = currentAnimation;
        xSemaphoreGive(configMutex);
    }
    else {
        ESP_LOGI("LED", "Failed to get current animation");
    }
    return animation;
}
void LedHandler::setCurrentAnimation(animationEnum animation) {
    if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
        currentAnimation = animation;
        xSemaphoreGive(configMutex);
    }
}