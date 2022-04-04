#include "PT7C4339.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#define TAG __PRETTY_FUNCTION__

PT7C4339 *rtc_4339;

#define LEAP_YEAR(_year) ((_year%4)==0)
#define DEC2BCD(val) ((val/10*16) + (val%10))
#define BCD2DEC(val) ((val/16*10) + (val%16))

StaticSemaphore_t PT7C4339::m_rtcMutexBuf;
SemaphoreHandle_t PT7C4339::m_rtcMutex;

/**************************************************************/
/* This class manages the PT7C4339 RTC Clock chip. This class */
/* will force the chip into the 24 hour mode of operation.    */
/* Time zone management is out of scope this class; it is     */
/* assumed the RTC and the system time is set to UTC          */
/**************************************************************/
PT7C4339::PT7C4339(TrickleCharge_t trickle, SquareWave_t square_wave) :
   I2CPort(I2CPort::OTHER, ADDRESS_0, 0xff),
   m_trickleCharge(trickle),
   m_squareWave(square_wave),
   m_haveNewTime(false)
{
   m_rtcMutex = xSemaphoreCreateMutexStatic(&m_rtcMutexBuf);
   xSemaphoreGive(m_rtcMutex);

   uint8_t regs[32];
   readRegister(ADDRESS_0, REG_SECS, regs, 17);
   uint8_t *rp = regs;
   for (int x = 0; x < 17; x++)
   {
      ESP_LOGI(TAG, "buf[%02d] = 0x%02x, %d", x, *rp, *rp);
      rp++;
   }

   /***********************************************************/
   /* We are using the Alarm 2 Date/Day registor to tell      */
   /* if the RTC has ever been configured. If not we will     */
   /* set the time registers to 0 and the alarm 2 data/day    */
   /* register to some arbitrary non-zero value               */
   /***********************************************************/
   if (regs[REG_ALM2_HOURS] == 0)
   {
      ESP_LOGI(TAG, "RTC Not configured");
      write(REG_ALM2_HOURS, 0x07);
      updateRTCTime(0);
//      write(REG_SECS, 0x00);
//      write(REG_MINS, 10);
//      write(REG_HOURS, 16);
//      write(REG_DAYS, 27);
//      write(REG_MONTHS, 03);
//      write(REG_YEARS, 22);
   }
   write(REG_CONTROL, square_wave);
   write(REG_STATUS, OSF);
   write(REG_TRICKLE, trickle);
}


/************************************************************/
/* This function reads the time from the RTC and calculates */
/* the seconds from epoc and saves it to the system clock   */
/************************************************************/
time_t PT7C4339::readRTCTime()
{
   uint8_t regs[32];
   time_t seconds = 0;
   struct tm cur_time;

   readRegister(ADDRESS_0, REG_SECS, regs, 17);

   // Lengthy way to get to then end, but it is explicate and it works
   int sec = regs[REG_SECS] & MSK_SECS;
   int min = regs[REG_MINS] & MSK_MINS;
   int hour = regs[REG_HOURS] & MSK_HOURS;
   int mday = regs[REG_DAYS] & MSK_DAYS;
   int mon = regs[REG_MONTHS] & MSK_MONTHS;
   int year = regs[REG_YEARS] & MSK_YEARS;
   cur_time.tm_sec = BCD2DEC(sec); // & MSK_SECS;
   cur_time.tm_min = BCD2DEC(min); // & MSK_MINS;
   cur_time.tm_hour = BCD2DEC(hour); // & MSK_HOURS;
   cur_time.tm_mday = BCD2DEC(mday);
   cur_time.tm_mon = BCD2DEC(mon) - 1; // & MSK_MONTHS) - 1;
   cur_time.tm_year = BCD2DEC(year) + 100;
   cur_time.tm_isdst = 0;
#if 0
   ESP_LOGI(TAG, "sec: 0x%02x, %d, 0x%02x", sec, cur_time.tm_sec, cur_time.tm_sec);
   ESP_LOGI(TAG, "min: 0x%02x, %d, 0x%02x", min, cur_time.tm_min, cur_time.tm_min);
   ESP_LOGI(TAG, "hour: 0x%02x, %d, 0x%02x", hour, cur_time.tm_hour, cur_time.tm_hour);
   ESP_LOGI(TAG, "mday: 0x%02x, %d, 0x%02x", mday, cur_time.tm_mday, cur_time.tm_mday);
   ESP_LOGI(TAG, "mon: 0x%02x, %d, 0x%20x", mon, cur_time.tm_mon, cur_time.tm_mon);
   ESP_LOGI(TAG, "year: 0x%02x, %d, 0x%02x", year, cur_time.tm_year, cur_time.tm_year);
#endif
   seconds = mktime(&cur_time);
   return(seconds);
}

void PT7C4339::updateRTCTime(time_t seconds)
{
   struct tm *cur_time;

   if (xSemaphoreTake(m_rtcMutex, 50 / portTICK_RATE_MS))
   {
      cur_time = gmtime(&seconds);
      if (cur_time != NULL)
      {
         int sec = (int)DEC2BCD(cur_time->tm_sec); 
         int min = (int)DEC2BCD(cur_time->tm_min);
         int hour = (int)DEC2BCD(cur_time->tm_hour);
         int wday = (int)DEC2BCD((cur_time->tm_wday + 1));
         int mday = (int)DEC2BCD(cur_time->tm_mday);
         int mon = (int)DEC2BCD((cur_time->tm_mon + 1));
         int year = (int)DEC2BCD((cur_time->tm_year %100));

         ESP_LOGI(TAG, "sec: %d, 0x%02x", cur_time->tm_sec, sec);
         m_time_queue.push_back(std::make_pair((int)REG_SECS, sec));
         ESP_LOGI(TAG, "min: %d, 0x%02x", cur_time->tm_min, min);
         m_time_queue.push_back(std::make_pair((int)REG_MINS, min));
         ESP_LOGI(TAG, "hour: %d, 0x%02x", cur_time->tm_hour, hour);
         m_time_queue.push_back(std::make_pair((int)REG_HOURS, hour));
         ESP_LOGI(TAG, "wday: %d, 0x%02x", cur_time->tm_wday, wday);
         m_time_queue.push_back(std::make_pair((int)REG_DOW, wday));
         ESP_LOGI(TAG, "mday: %d, 0x%02x", cur_time->tm_mday, mday);
         m_time_queue.push_back(std::make_pair((int)REG_DAYS, mday));
         ESP_LOGI(TAG, "mon: %d, 0x%02x", cur_time->tm_mon, mon);
         m_time_queue.push_back(std::make_pair((int)REG_MONTHS, mon));
         ESP_LOGI(TAG, "year: %d, 0x%02x", cur_time->tm_year, year);
         m_time_queue.push_back(std::make_pair((int)REG_YEARS, year));
      }
      xSemaphoreGive(m_rtcMutex);
   }
   else
   {
      ESP_LOGE(TAG, "Semaphore Timeout!");
   } 
}

void PT7C4339::processPort(uint8_t addr, uint8_t ddr)
{
   if (xSemaphoreTake(m_rtcMutex, 50 / portTICK_RATE_MS))
   {
      if (! m_time_queue.empty())
      {
         time_entry_t te = m_time_queue.front();
         m_time_queue.pop_front();
         write(te.first, te.second);
      }
      xSemaphoreGive(m_rtcMutex);
   }
   else
   {
      ESP_LOGE(TAG, "Semaphore Timeout!");
   } 
}

