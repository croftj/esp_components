# include "I2CPort.h"

#include <thread>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG __PRETTY_FUNCTION__

#define IO_LOOP_MS  1
#define INTER_IO_US 2

#define WRITE_BIT                          I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                           I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN                       0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                      0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                            ((i2c_ack_type_t)0x0)              /*!< I2C ack value */
#define NACK_VAL                           ((i2c_ack_type_t)0x1)              /*!< I2C nack value */

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
//   ESP_LOGI(TAG, "m_pins[0x%02x] = 0x%02x", (int)m_addr, (int)m_pins[m_addr]);
//      Serial << __FUNCTION__ << " port: " << (int)port << ", m_pins: 0x" << itoa(m_pins[port], ibuf, 16) << endl;
//      writePortData(m_addr, m_pins[m_addr]);
}

esp_err_t I2CPort::readRegister(uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_rd, size_t size)
{
   esp_err_t rv = 0;
   if (xSemaphoreTake(m_ioMutex, 10 / portTICK_RATE_MS))
   {
      if (size == 0)
      {
         return ESP_OK;
      }
      i2c_cmd_handle_t cmd = i2c_cmd_link_create();
      i2c_master_start(cmd);
      // first, send device address (indicating write) & register to be read
      i2c_master_write_byte(cmd, ( i2c_addr << 1 ), ACK_CHECK_EN);
      // send register we want
      ESP_LOGI(TAG, "reading %d bytes from addr 0x%2x, starting at reg %d", size, i2c_addr, i2c_reg);
      i2c_master_write_byte(cmd, i2c_reg, ACK_CHECK_EN);
      // Send repeated start
      i2c_master_start(cmd);
      // now send device address (indicating read) & read data
      i2c_master_write_byte(cmd, ( i2c_addr << 1 ) | READ_BIT, ACK_CHECK_EN);
      if (size > 1)
      {
         i2c_master_read(cmd, data_rd, size - 1, ACK_VAL);
      }
      i2c_master_read_byte(cmd, data_rd + size - 1, NACK_VAL);
      i2c_master_stop(cmd);
      rv = i2c_master_cmd_begin(m_port, cmd, 1000 / portTICK_RATE_MS);
      i2c_cmd_link_delete(cmd);
      xSemaphoreGive(m_ioMutex);
   }
   else
      ESP_LOGW(TAG, "I2C Semaphore timeout");
   ESP_LOGW(TAG, "rv = %d", (int)rv);
   return rv;
}

bool I2CPort::write(uint8_t reg, uint8_t data)
{
   esp_err_t err;
   int rv = 0;

   if (xSemaphoreTake(m_ioMutex, 10 / portTICK_RATE_MS))
   {
      ESP_LOGW(TAG, "got I2C semaphore");
      i2c_cmd_handle_t cmd = i2c_cmd_link_create();
      i2c_master_start(cmd);
      i2c_master_write_byte(cmd, m_addr << 1| I2C_MASTER_WRITE, (i2c_ack_type_t)1);
      i2c_master_write_byte(cmd, reg, (i2c_ack_type_t)1);
      i2c_master_write_byte(cmd, data, (i2c_ack_type_t)1);
      i2c_master_stop(cmd);
//      ESP_LOGI(DEVICE_NAME, "writing: 0x%x to reg 0x%x at addr 0x%x", data, reg, m_addr);
      if ((err = i2c_master_cmd_begin(m_port, cmd, 1000 / portTICK_RATE_MS)) != ESP_OK)
      {
         if (err == ESP_ERR_TIMEOUT)
            ESP_LOGW(TAG, "I2C Bus is busy");
         else
            ESP_LOGW(TAG, "I2C Write Failed: a:0x%x, d:0x%x", m_addr, data);
         rv = -1;
      }
      i2c_cmd_link_delete(cmd);
      ESP_LOGW(TAG, "returned I2C semaphore");
      xSemaphoreGive(m_ioMutex);
   }
   else
   {
      ESP_LOGW(TAG, "I2C Semaphore timeout");
      rv = -1;
   }
   return(rv);
}

void I2CPort::exec(void*)
{
   std::this_thread::sleep_for(std::chrono::seconds(5));
   while (true)
   {
//      ESP_LOGI(TAG, "top of loop");
      int cnt = 0;
//      for (int addr = 0; addr < NUM_PORTS && cnt < 3; addr++)
      gpio_set_level(IO1_LED_PIN, 1);
      for (int addr = 0; addr < NUM_PORTS; addr++)
      {
//         ESP_LOGI(TAG, "port #%d", addr);
         if (m_outputEnabled && m_defaultPort != NULL )
         {
            if (m_mode[addr] == OUTPUT)
            {
               gpio_set_level(IO2_LED_PIN, 1);
               if (true || m_pinsFirst || m_pinsPrevious[addr] != m_pins[addr])
               {
//                  ESP_LOGI(TAG, "writing port: 0x%02x = 0x%02x", (int)addr, (int)m_pins[addr]);
                  m_objects[addr]->writePortData(addr, m_pins[addr]);
                  m_pinsPrevious[addr] = m_pins[addr];
//                  std::this_thread::sleep_for(std::chrono::microseconds(INTER_IO_US));
               }
               gpio_set_level(IO2_LED_PIN, 0);
               cnt++;
            }
            else if (true && m_mode[addr] != UNDEF)
            {
//               ESP_LOGI(TAG, "processing port: 0x%02x = 0x%02x", (int)addr, (int)m_pins[addr]);
               m_objects[addr]->processPort(addr, m_ddr[addr]);
//               std::this_thread::sleep_for(std::chrono::microseconds(INTER_IO_US));
            }
         }
      }
      gpio_set_level(IO1_LED_PIN, 0);
      m_pinsFirst = false;
//      ESP_LOGI(TAG, "bottom of loop");
//      std::this_thread::sleep_for(std::chrono::milliseconds(IO_LOOP_MS));
   }
   return;
}


