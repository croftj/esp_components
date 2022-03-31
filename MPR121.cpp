#include "MPR121.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/semphr.h"

#define DEVICE_NAME "MPR121"
#define TAG __PRETTY_FUNCTION__

//StaticSemaphore_t MPR121::m_ioMutexBuf;
//SemaphoreHandle_t MPR121::m_ioMutex;

MPR121 *mpr121;

MPR121::MPR121(I2CPort::IOMode_t mode, uint8_t addr, uint8_t mask, gpio_num_t irq_pin) :
      I2CPort(mode, addr, mask),
      m_keyStatus(0x00),
      m_irqPin(irq_pin),
      m_CDC(15),
      m_FFI(0),
      m_CDT(1),
      m_SFI(0),
      m_ESI(4),
      m_TTH(0x3f),
      m_RTH(0x2a)
{
   gpio_config_t kbdio_conf;
   kbdio_conf.intr_type = GPIO_INTR_DISABLE;
   kbdio_conf.mode = GPIO_MODE_INPUT;
   //bit mask of the pins that you want to set,e.g.GPIO18/19
   kbdio_conf.pin_bit_mask = 1ULL << m_irqPin;
   kbdio_conf.pull_down_en = (gpio_pulldown_t)0;
   kbdio_conf.pull_up_en = (gpio_pullup_t)1;
   gpio_config(&kbdio_conf);

   // Section A
   // This group controls filtering when data is > baseline.
   write(SRR, SOFT_RESET);

   write(MHD_R, 0x01);
   write(NHD_R, 0x01);
   write(NCL_R, 0x00);
   write(FDL_R, 0x00);

   // Section B
   // This group controls filtering when data is < baseline.
   write(MHD_F, 0x01);
   write(NHD_F, 0x01);
   write(NCL_F, 0xFF);
   write(FDL_F, 0x02);

   // Section C
   // This group sets touch and release thresholds for each electrode
   write(ELE0_T, m_TTH);
   write(ELE0_R, m_RTH);
   write(ELE1_T, m_TTH);
   write(ELE1_R, m_RTH);
   write(ELE2_T, m_TTH);
   write(ELE2_R, m_RTH);
   write(ELE3_T, m_TTH);
   write(ELE3_R, m_RTH);
   write(ELE4_T, m_TTH);
   write(ELE4_R, m_RTH);
   write(ELE5_T, m_TTH);
   write(ELE5_R, m_RTH);

   write(ELE6_T, m_TTH);
   write(ELE6_R, m_RTH);
   write(ELE7_T, m_TTH);
   write(ELE7_R, m_RTH);
   write(ELE8_T, m_TTH);
   write(ELE8_R, m_RTH);
   write(ELE9_T, m_TTH);
   write(ELE9_R, m_RTH);
   write(ELE10_T, m_TTH);
   write(ELE10_R, m_RTH);
   write(ELE11_T, m_TTH);
   write(ELE11_R, m_RTH);

   // Section D
   // Set the Filter Configuration
   // Set ESI2
//      write(AFE_2, 0x04);
//      write(AFE_2, 0x24);
   write(AFE_1, m_FFI << 6 | m_CDC << 5);
   write(AFE_2, m_CDT << 5 | m_SFI << 3 | m_ESI << 0);

   // Section E
   // Electrode Configuration
   // Enable 6 Electrodes and set to run mode
   // Set ELE_CFG to 0x00 to return to standby mode
   // write(ELE_CFG, 0x0C);   // Enables all 12 Electrodes
   write(ELE_CFG, 0x07);      // Enable first 6 electrodes

   // Section F
   // Enable Auto Config and auto Reconfig
/*
   write(ATO_CFG0, 0x0B);
   write(ATO_CFGU, 0xC9);     // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V
   write(ATO_CFGL, 0x82);     // LSL = 0.65*USL = 0x82 @3.3V
   write(ATO_CFGT, 0xB5);     // Target = 0.9*USL = 0xB5 @3.3V
*/
}

void MPR121::processPort(uint8_t addr, uint8_t ddr) 
{
   esp_err_t err;
   uint8_t buf[3];

   int irq = gpio_get_level(m_irqPin); 
   if ( ! irq)
   {
      ESP_LOGI(DEVICE_NAME, "Detected Keypress!");
      if (xSemaphoreTake(m_ioMutex, 10 / portTICK_RATE_MS))
      {
         ESP_LOGW(TAG, "got I2C semaphore");
         i2c_cmd_handle_t cmd = i2c_cmd_link_create();
         i2c_master_start(cmd);
//         i2c_master_write_byte(cmd, m_addr << 1| I2C_MASTER_WRITE, (i2c_ack_type_t)1);
//         i2c_master_write_byte(cmd, TOUCH_STATUS, (i2c_ack_type_t)1);
//         i2c_master_start(cmd);
         i2c_master_write_byte(cmd, m_addr << 1| I2C_MASTER_READ, (i2c_ack_type_t)1);
         i2c_master_read_byte(cmd, buf, (i2c_ack_type_t)1);
         i2c_master_read_byte(cmd, buf + 1, (i2c_ack_type_t)0);
         i2c_master_stop(cmd);
         if ((err = i2c_master_cmd_begin(m_port, cmd, 5 / portTICK_RATE_MS)) != ESP_OK)
         {
            if (err == ESP_ERR_TIMEOUT)
               ESP_LOGW(DEVICE_NAME, "I2C Bus is busy");
            else
               ESP_LOGW(DEVICE_NAME, "I2C eead Failed: a:0x%x", addr);
         }
         i2c_cmd_link_delete(cmd);
         ESP_LOGW(TAG, "returned I2C semaphore");
         xSemaphoreGive(m_ioMutex);
         ESP_LOGI(DEVICE_NAME, "buf = 0x%04x", *((uint16_t*)buf));
         m_keyStatus = (0x00ff & buf[0]);
         ESP_LOGI(DEVICE_NAME, "m_keyStatus = 0x%04x", m_keyStatus);
      }
      else
         ESP_LOGW(DEVICE_NAME, "I2C Semaphore timeout");
   }
}

void MPR121::writePortData(uint8_t addr, uint8_t data) 
{
}

void MPR121::writePortDDR(uint8_t addr, uint8_t ddr)
{
}

void MPR121::initializePort(uint8_t addr, uint8_t ddr)
{
   // Section A
   // This group controls filtering when data is > baseline.
   write(SRR, SOFT_RESET);

   write(MHD_R, 0x01);
   write(NHD_R, 0x01);
   write(NCL_R, 0x00);
   write(FDL_R, 0x00);

   // Section B
   // This group controls filtering when data is < baseline.
   write(MHD_F, 0x01);
   write(NHD_F, 0x01);
   write(NCL_F, 0xFF);
   write(FDL_F, 0x02);

   // Section C
   // This group sets touch and release thresholds for each electrode
   write(ELE0_T, m_TTH);
   write(ELE0_R, m_RTH);
   write(ELE1_T, m_TTH);
   write(ELE1_R, m_RTH);
   write(ELE2_T, m_TTH);
   write(ELE2_R, m_RTH);
   write(ELE3_T, m_TTH);
   write(ELE3_R, m_RTH);
   write(ELE4_T, m_TTH);
   write(ELE4_R, m_RTH);
   write(ELE5_T, m_TTH);
   write(ELE5_R, m_RTH);

   write(ELE6_T, m_TTH);
   write(ELE6_R, m_RTH);
   write(ELE7_T, m_TTH);
   write(ELE7_R, m_RTH);
   write(ELE8_T, m_TTH);
   write(ELE8_R, m_RTH);
   write(ELE9_T, m_TTH);
   write(ELE9_R, m_RTH);
   write(ELE10_T, m_TTH);
   write(ELE10_R, m_RTH);
   write(ELE11_T, m_TTH);
   write(ELE11_R, m_RTH);

   // Section D
   // Set the Filter Configuration
   // Set ESI2
//      write(AFE_2, 0x04);
//      write(AFE_2, 0x24);
   write(AFE_1, m_FFI << 6 | m_CDC << 5);
   write(AFE_2, m_CDT << 5 | m_SFI << 3 | m_ESI << 0);

   // Section E
   // Electrode Configuration
   // Enable 6 Electrodes and set to run mode
   // Set ELE_CFG to 0x00 to return to standby mode
   // write(ELE_CFG, 0x0C);   // Enables all 12 Electrodes
//   write(ELE_CFG, 0x0c);      // Enable first 6 electrodes
   write(ELE_CFG, 0x00);      // Enable first 6 electrodes

   // Section F
   // Enable Auto Config and auto Reconfig
/*
   write(ATO_CFG0, 0x0B);
   write(ATO_CFGU, 0xC9);     // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V
   write(ATO_CFGL, 0x82);     // LSL = 0.65*USL = 0x82 @3.3V
   write(ATO_CFGT, 0xB5);     // Target = 0.9*USL = 0xB5 @3.3V
*/
}
