#ifndef I2CPORT_H
#define I2CPORT_H

#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include <set>

#ifndef _BV
#define _BV(pin) (1 << pin)
#endif

class I2CPort
{
public:
   typedef enum
   {
      INPUT = 0,
      OUTPUT
   } IOMode_t;

   I2CPort(IOMode_t mode, uint8_t addr, uint8_t mask);
   void setPort(uint8_t pins);

   void initialize(i2c_port_t i2c_port, uint8_t sda_io, uint8_t scl_io, uint32_t clk_speed)
   {
      m_defaultPort = this;
      m_port = i2c_port;
      i2c_config_t conf;

      m_ioMutex = xSemaphoreCreateMutexStatic(&m_ioMutexBuf);
      xSemaphoreGive(m_ioMutex);

      conf.mode = I2C_MODE_MASTER;
      conf.master.clk_speed = clk_speed;
      conf.sda_io_num    = (gpio_num_t)sda_io;
      conf.scl_io_num    = (gpio_num_t)scl_io;
      conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
      conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
      conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
      i2c_driver_install(i2c_port, I2C_MODE_MASTER, 0, 0, 0);
      if (i2c_param_config(i2c_port, &conf) == ESP_OK)
      {
         ESP_LOGI("I2CPort", "Port configured");
         for (int x = 0; x < NUM_PORTS; x++)
         {
            if (m_outputAddresses[x] > 0)
            {
               writePortDDR(m_outputAddresses[x], m_ddr[x]);
            }
         }
      }
      else
      {
         ESP_LOGI("I2CPort", "Port configuration failed");
      } 
      m_initialized = true;
   }

   void on()
   {
      setPort(m_mask);
   }

   void off()
   {
      clearPort(m_mask);
   }

   void set(uint8_t value)
   {
      value &= m_mask;
      m_pins[m_addr] &= ~value;
      m_pins[m_addr] |= value;
//      Serial << __FUNCTION__ << " port: " << (int)m_addr << ", m_addrregs: " << (int) m_addrregs[m_addr] << endl;
//      writePortData(m_addr, m_pins[m_addr]);
   }

   uint8_t read()
   {
      return(readPortData(m_addr) & m_mask);
   }

   void setPin(uint8_t pin)
   {
      m_pins[m_addr] |= _BV(pin);
//      char ibuf[8];
//      Serial << __FUNCTION__ << " port: " << (int)port << ", m_pins: 0x" << itoa(m_pins[port], ibuf, 16) << endl;
//      writePortData(m_addr, m_pins[m_addr]);
   }

   void clearPin(uint8_t pin)
   {
      m_pins[m_addr] &= ~(_BV(pin));
//      char iTuf[8];
//      Serial << __FUNCTION__ << " port: " << (int)port << ", m_pins: 0x" << itoa(m_pins[port], ibuf, 16) << endl;
//      writePortData(m_addr, m_pins[m_addr]);
   }

   void clearPort(uint8_t pins)
   {
      m_pins[m_addr] &= ~pins;
//      char ibuf[8];
//      Serial << __FUNCTION__ << " port: " << (int)port << ", m_pins: 0x" << itoa(m_pins[port], ibuf, 16) << endl;
//      writePortData(m_addr, m_pins[m_addr]);
   }

   bool readPin(uint8_t pin)
   {
      return(readPortData(m_addr) & _BV(pin));
   }

   uint8_t readPort()
   {
      return(readPortData(m_addr));
   }

   void setPortDDR(uint8_t ddr)
   {
      m_ddr[m_addr] |= ddr;
//      Serial << __FUNCTION__ << " port: " << (int)port << ", m_ddr: " << (int) m_ddr[port] << endl;
//      writePortDDR(m_addr, m_ddr[m_addr]);
   }

   void setPinDDR(uint8_t pin, bool ddr)
   {
      if ( ddr ) 
      {
         m_ddr[m_addr] |= _BV(pin);
      }
      else 
      {
         m_ddr[m_addr] &= ~(_BV(pin));
      }
//      Serial << __FUNCTION__ << " port: " << (int)port << ", m_ddr: " << (int) m_ddr[port] << endl;
//      writePortDDR(m_addr, m_ddr[m_addr]);
   }

   void clearPortDDR(uint8_t ddr)
   {
      m_ddr[m_addr] &= ~(ddr);
//      writePortDDR(m_addr, m_ddr[m_addr]);
   }

   void static setOutputEnable(bool f)
   {
      m_outputEnabled = f;
   }

   void static *exec(void*);

protected:
   virtual uint8_t  readPortData(uint8_t m_addr) = 0;
   virtual void     writePortData(uint8_t m_addr, uint8_t data) = 0;
   virtual void     writePortDDR(uint8_t m_addr, uint8_t ddr) = 0;

   uint8_t  m_addr;
   uint8_t  m_mask;               

   static I2CPort*            m_defaultPort;
   static i2c_port_t          m_port;
   static bool                m_initialized;
   static bool                m_outputEnabled;
   static StaticSemaphore_t   m_ioMutexBuf;
   static SemaphoreHandle_t   m_ioMutex;

private:
   enum {
      NUM_PORTS = 256
   };
   
   static uint8_t             m_outputAddresses[NUM_PORTS];
   static uint8_t             m_ddr[NUM_PORTS];
   static uint8_t             m_pins[NUM_PORTS];
};

#endif

