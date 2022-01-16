#include "freertos/FreeRTOS.h"
#include "esp8266_peri.h"

#define NOP() asm volatile ("nop")

unsigned long micros()
{
    return (unsigned long) (esp_timer_get_time());
}
void delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if(us){
        uint32_t e = (m + us);
        if(m > e){ //overflow
            while(micros() > e){
                NOP();
            }
        }
        while(micros() < e){
            NOP();
        }
    }
}
