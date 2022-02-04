#include "MAX7320.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define DEVICE_NAME "MAX7320"

MAX7320 dummy_port(I2CPort::OUTPUT, 0, 0);

void MAX7320::processPort(uint8_t addr, uint8_t ddr) 
{
}

void MAX7320::writePortData(uint8_t addr, uint8_t data) 
{
   esp_err_t err;

   if (m_initialized)
   {
      if (xSemaphoreTake(m_ioMutex, 1000 / portTICK_RATE_MS))
      {
         i2c_cmd_handle_t cmd = i2c_cmd_link_create();
         i2c_master_start(cmd);
         i2c_master_write_byte(cmd, addr << 1| I2C_MASTER_WRITE, (i2c_ack_type_t)1);
         i2c_master_write_byte(cmd, data, (i2c_ack_type_t)1);
         i2c_master_stop(cmd);
         if ((err = i2c_master_cmd_begin(m_port, cmd, 1000 / portTICK_RATE_MS)) != ESP_OK)
   //      if ((err = i2c_master_cmd_begin(m_port, cmd, 1)) != ESP_OK)
         {
            if (err == ESP_ERR_TIMEOUT)
               ESP_LOGW(DEVICE_NAME, "I2C Bus is busy");
            else
               ESP_LOGW(DEVICE_NAME, "I2C Write Failed: a:0x%x, d:0x%x", addr, data);
         }
         i2c_cmd_link_delete(cmd);
         xSemaphoreGive(m_ioMutex);
      }
      else
         ESP_LOGW("I2C_Write", "I2C Semaphore taken");
   }
   else
      ESP_LOGW("I2C_Write", "I2C Bus not initialized");
}

void MAX7320::writePortDDR(uint8_t addr, uint8_t ddr)
{
}

void MAX7320::initializePort(uint8_t addr, uint8_t ddr)
{
}
