# include "I2CPort.h"

#include <thread>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG __PRETTY_FUNCTION__

#define IO_LOOP_MS  20
#define INTER_IO_US 50

I2CPort* I2CPort::m_defaultPort = NULL;

bool              I2CPort::m_initialized   = false;
i2c_port_t        I2CPort::m_port          = 0;
bool              I2CPort::m_outputEnabled = true;
bool              I2CPort::m_pinsFirst     = true;

StaticSemaphore_t I2CPort::m_ioMutexBuf;
SemaphoreHandle_t I2CPort::m_ioMutex;

uint8_t I2CPort::m_mode[NUM_PORTS] =
{
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0
};

uint8_t I2CPort::m_ddr[NUM_PORTS] =
{
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0
};

uint8_t I2CPort::m_pinsPrevious[NUM_PORTS] =
{
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0
};

uint8_t I2CPort::m_pins[NUM_PORTS] =
{
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0
};


I2CPort* I2CPort::m_objects[NUM_PORTS] =
{
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

#define IO1_LED_PIN  ((gpio_num_t)14)
#define IO2_LED_PIN  ((gpio_num_t)15)
#define GPIO_IO_MASK ((1ULL<<IO1_LED_PIN) | (1ULL<<IO2_LED_PIN))
I2CPort::I2CPort(IOMode_t mode, uint8_t addr, uint8_t mask) :
   m_addr(addr),
   m_mask(mask)
{
   if (addr > 0)
   {
      gpio_config_t iop_conf;
      m_mode[m_addr] = mode;
      m_objects[m_addr] = this;
      ESP_LOGI(TAG, "m_mode[0x%02x] = 0x%02x", (int)m_addr, (int)m_mode[m_addr]);
      if (mode == OUTPUT)
      {
         m_ddr[m_addr] |= mask;
      }
      else if (mode == INPUT)
      {
         m_ddr[m_addr] &= ~mask;
      }
      iop_conf.intr_type = GPIO_INTR_DISABLE;
      iop_conf.mode = GPIO_MODE_OUTPUT;
      //bit mask of the pins that you want to set,e.g.GPIO18/19
      iop_conf.pin_bit_mask = GPIO_IO_MASK;
      iop_conf.pull_down_en = (gpio_pulldown_t)0;
      iop_conf.pull_up_en = (gpio_pullup_t)0;
      gpio_config(&iop_conf);
      gpio_set_level(IO1_LED_PIN, 0);
      gpio_set_level(IO2_LED_PIN, 0);
   }
}

void I2CPort::setPort(uint8_t pins)
{
//      char ibuf[8];
   m_pins[m_addr] |= pins;
   ESP_LOGI(TAG, "m_pins[0x%02x] = 0x%02x", (int)m_addr, (int)m_pins[m_addr]);
//      Serial << __FUNCTION__ << " port: " << (int)port << ", m_pins: 0x" << itoa(m_pins[port], ibuf, 16) << endl;
//      writePortData(m_addr, m_pins[m_addr]);
}

void* I2CPort::exec(void*)
{
   std::this_thread::sleep_for(std::chrono::seconds(5));
   while (true)
   {
//      ESP_LOGI(TAG, "top of loop");
      int cnt = 0;
//      for (int addr = 0; addr < NUM_PORTS && cnt < 3; addr++)
      gpio_set_level(IO1_LED_PIN, 1);
      for (int addr = 0; addr < NUM_PORTS && cnt < 3; addr++)
      {
//         ESP_LOGI(TAG, "port #%d", addr);
         if (m_outputEnabled && m_defaultPort != NULL )
         {
            if (m_mode[addr] == OUTPUT)
            {
               if (true || m_pinsFirst || m_pinsPrevious[addr] != m_pins[addr])
               {
//                  ESP_LOGI(TAG, "writing port: 0x%02x = 0x%02x", (int)addr, (int)m_pins[addr]);
                  gpio_set_level(IO2_LED_PIN, 1);
                  m_objects[addr]->writePortData(addr, m_pins[addr]);
                  m_pinsPrevious[addr] = m_pins[addr];
                  gpio_set_level(IO2_LED_PIN, 0);
//                  std::this_thread::sleep_for(std::chrono::microseconds(INTER_IO_US));
                  cnt++;
               }
            }
            else if (true && m_mode[addr] != UNDEF)
            {
//               ESP_LOGI(TAG, "processing port: 0x%02x = 0x%02x", (int)addr, (int)m_pins[addr]);
               m_objects[addr]->processPort(addr, m_ddr[addr]);
               std::this_thread::sleep_for(std::chrono::microseconds(INTER_IO_US));
            }
         }
      }
      gpio_set_level(IO1_LED_PIN, 0);
      m_pinsFirst = false;
//      ESP_LOGI(TAG, "bottom of loop");
//      std::this_thread::sleep_for(std::chrono::milliseconds(IO_LOOP_MS));
   }
   return(NULL);
}


