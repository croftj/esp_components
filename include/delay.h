#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void delay(uint32_t ms)
{
   vTaskDelay((int)((double)ms / 1.0 / CONFIG_FREERTOS_HZ * 1000.0));
}
