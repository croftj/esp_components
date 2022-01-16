#ifndef IOPORT_H
#define IOPORT_H

#ifdef ARDUINO
# include <avr/common.h>
# include <avr/pgmspace.h>
# include <avr/io.h>
#else
# include "driver/gpio.h"
#endif /* Arduino */

#include <iostream>
# include <stdint.h>
# include <stdio.h>
# include <vector>

class IOPort
{
public:
   IOPort()
   {
     m_config = new(std::vector<gpio_config_t*>); 
   };

   void on()
   {
      set(m_mask);
   }

   void off()
   {
      set(0ULL);
   }

   void set(uint64_t value)
   {
      value &= m_mask;
//      std::cout << "set(): here- m_mask: " << (int)m_mask << ", sizeof(val): " << (int)sizeof(value) << std::endl;
      for (int pin = 0; pin < sizeof(value) * 8; pin++)
      {
//         std::cout << "set(): pin: " << pin << ", val: " << (uint64_t)value << ", mask: " << (uint64_t)(1ULL << pin) 
//                   << ", and: " << (uint64_t)(value & (1ULL << pin)) << std::endl;
         if ((m_mask & (uint64_t)(1ULL << pin)) != 0)
         {
            uint32_t level = ((value & (uint64_t)(1ULL << pin)) != 0) ? 1 : 0;
//            std::cout << "set(): pin: " << pin << ", val: " << level << std::endl;
            gpio_set_level((gpio_num_t)pin, level);
         }
      }
   }

   void setPin(uint8_t pin, uint32_t level)
   {
      if ((m_mask & (uint64_t)(1ULL << pin)) != 0)
      {
//         std::cout << "set(): pin: " << pin << ", val: " << (int)level << std::endl;
         gpio_set_level((gpio_num_t)pin, level);
      }
   }

   /*************************************************************/
   /* configPort for the ESP32 (Non-Arduino) breaks compatibility  */
   /* with the Arduino fiunctionality.                          */
   /*                                                           */
   /* Whereas the arduino version sets the port values,         */
   /* The ESP32 version has the same functionality as the       */
   /* configPort function for InputPort and OuputPort where it     */
   /* configures the port definition.                           */
   /*                                                           */
   /* This function returns a pointer to the port configuration */
   /*************************************************************/
   gpio_config_t* configPort(uint64_t mask, gpio_mode_t mode
                             , gpio_pullup_t pull_up_en = GPIO_PULLUP_DISABLE
                             , gpio_pulldown_t pull_down_en = GPIO_PULLDOWN_DISABLE
                             , gpio_int_type_t intr_type = GPIO_INTR_DISABLE)
   {
      m_port = 0;
      m_mask = mask;
      gpio_config_t* config = NULL;
      int cfg_size = m_config->size();
      for (int x = 0; x < cfg_size; x++)
      {
         if (m_config->at(x)->mode == mode
          && m_config->at(x)->pull_up_en == pull_up_en
          && m_config->at(x)->pull_down_en == pull_down_en
          && m_config->at(x)->intr_type == intr_type)
         {
            config = m_config->at(x);
            break;
         }
      }
      if (config == NULL)
      {
         m_config->resize(m_config->size() + 1);
         config = new gpio_config_t;
         config->mode = mode;
         config->pull_up_en = pull_up_en;
         config->pull_down_en = pull_down_en;
         config->intr_type = intr_type;
         config->pin_bit_mask = 0;
         m_config->push_back(config);
      }
      config->pin_bit_mask |= mask; 
      gpio_config(config);
      m_portConfig = config;
      return(config);
   }

   uint64_t readPin(uint8_t pin)
   {
      if ((m_mask & (1ULL << pin)) == 1)
      {
         return(gpio_get_level((gpio_num_t)pin));
      }
      return(0);
   }

   enum {
      INPUT = 0,
      OUTPUT
   };

   uint64_t read()
   {
      uint64_t rv    = 0;

      std::cerr << "here!" << std::endl;
      for (int pin = 0; pin < sizeof(m_mask) * 8; pin++)
      {
         if (readPin(pin) == 1)
         {
            rv |= 1ULL << pin;
         }
      }
      std::cerr << "done" << std::endl;
      return(rv);
   }

   uint64_t                   m_port;
   uint64_t                   m_mask;
   gpio_config_t*             m_portConfig;

   static void pinMode(unsigned int pin, uint8_t mode, bool pull_up = false)
   {
      gpio_mode_t m = (mode == INPUT) ? GPIO_MODE_INPUT_OUTPUT : GPIO_MODE_INPUT_OUTPUT;
//      std::cout << "IOPort::pinMode(): GPIO_MODE_INPUT = " << (int)GPIO_MODE_INPUT << ", GPIO_MODE_OUTPUT = " << (int)GPIO_MODE_OUTPUT << std::endl;
//      std::cout << "IOPort::pinMode(): pin = " << pin << ", mode << " << (int)mode << ", m = " << m << std::endl;
      gpio_set_direction((gpio_num_t)pin, m);
      if (mode == INPUT && pull_up) 
      {
         gpio_pullup_en((gpio_num_t)pin);
      }
      else
      {
         gpio_pullup_dis((gpio_num_t)pin);
      }
   }

private:
   static std::vector<gpio_config_t*>* m_config;
};

class OutputPort : public IOPort
{
public:
   void configPort(uint64_t mask, gpio_pullup_t pull_up_en = GPIO_PULLUP_DISABLE
                             , gpio_pulldown_t pull_down_en = GPIO_PULLDOWN_DISABLE
                             , gpio_int_type_t intr_type = GPIO_INTR_DISABLE)
   {
      IOPort::configPort(mask, GPIO_MODE_INPUT_OUTPUT
                          , pull_up_en
                          , pull_down_en
                          , intr_type);
   }

private:
protected:
};

class InputPort : public IOPort
{
public:
   void configPort(uint64_t mask, gpio_pullup_t pull_up_en = GPIO_PULLUP_DISABLE
                             , gpio_pulldown_t pull_down_en = GPIO_PULLDOWN_DISABLE
                             , gpio_int_type_t intr_type = GPIO_INTR_DISABLE)
   {
      IOPort::configPort(mask, GPIO_MODE_INPUT_OUTPUT
                          , pull_up_en
                          , pull_down_en
                          , intr_type);
   }

private:
protected:
};

#endif

