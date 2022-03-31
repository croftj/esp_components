#ifndef __PT7C4339_H
#define __PT7C4339_H

#include <deque>
#include <sys/time.h>
#include <utility> 

#include "I2CPort.h"

class PT7C4339 : public I2CPort
{
public:
   typedef enum {
      SQW_1             = 0x00,
      SQW_4K            = 0x08,
      SQW_8K            = 0x10,
      SQW_32K           = 0x18
   } SquareWave_t;

   typedef enum {
      TRICKLE_NONE      = 0xff,
      TRICKLE_200       = 0xa5,
      TRICKLE_200_DIODE = 0xa9,
      TRICKLE_2K        = 0xa5,
      TRICKLE_2K_DIODE  = 0xaa,
      TRICKLE_4K        = 0xa7,
      TRICKLE_4K_DIODE  = 0xab
   } TrickleCharge_t;

   typedef enum {
      CLOCK_12H         = 0x80,
      CLOCK_24H         = 0x00
   } Clock_t;

   typedef std::pair<int, int> time_entry_t;
   typedef std::deque<time_entry_t> time_queue_t;
   
   PT7C4339(TrickleCharge_t trickle, SquareWave_t square_wave);
   time_t readRTCTime();
   void updateRTCTime(time_t seconds);

   virtual void   processPort(uint8_t addr, uint8_t ddr);
   virtual void   writePortData(uint8_t addr, uint8_t data) {};
   virtual void   writePortDDR(uint8_t addr, uint8_t ddr)   {};
   virtual void   initializePort(uint8_t addr, uint8_t ddr) {};

protected:
   enum 
   {
      // I2C Address
      ADDRESS_0         = 0x68,

      // Registers
      REG_SECS          = 0x00,
      REG_MINS          = 0x01,
      REG_HOURS         = 0x02,
      REG_DOW           = 0x03,
      REG_DAYS          = 0x04,
      REG_MONTHS        = 0x05,
      REG_YEARS         = 0x06,
      REG_ALM1_SECS     = 0x07,
      REG_ALM1_MINS     = 0x08,
      REG_ALM1_HOURS    = 0x09,
      REG_ALM1_DAY      = 0x0a,
      REG_ALM2_MINS     = 0x0b,
      REG_ALM2_HOURS    = 0x0c,
      REG_ALM2_DAY      = 0x0d,
      REG_CONTROL       = 0x0e,
      REG_STATUS        = 0x0f,
      REG_TRICKLE       = 0x10,

      // Register Masks Etc
      MSK_AM_PM         = 0x40,
      MSK_HOURS         = 0x1F,
      MSK_DAYS          = 0x3F,
      MSK_MINS          = 0x7F,
      MSK_SECS          = 0x7F,
      MSK_DOW           = 0x07,
      MSK_MONTHS        = 0x1f,
      MSK_CENTURY       = 0x80,
      MSK_YEARS         = 0xff,

      OFS_AM            = 41,
      OFS_PM            = 61,

      // Control Bits
      EOSC              = 0x80,
      BBSQI             = 0x20,
      SQW_MASK          = 0x18,
      INTCN             = 0x04,
      A1IE              = 0x02,
      A2IE              = 0x01,

      // Status Bits
      OSF               = 0x80,
      A2F               = 0x01,
      A1F               = 0x02
   };

private:
   Clock_t           m_clockType;
   TrickleCharge_t   m_trickleCharge;
   SquareWave_t      m_squareWave;
   uint16_t          m_tzOffset;
   time_t            m_newTime;
   bool              m_haveNewTime;
   time_queue_t      m_time_queue;
   static StaticSemaphore_t m_rtcMutexBuf;
   static SemaphoreHandle_t m_rtcMutex;
};

extern PT7C4339 *rtc_4339;

#endif
