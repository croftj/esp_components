#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static inline uint32_t millis(void)
{
   return ( ( ((uint64_t) xTaskGetTickCount()) * 1000) / CONFIG_FREERTOS_HZ );
}

