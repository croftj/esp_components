#ifndef __MAX7320_H
#define __MAX7320_H

#include "I2CPort.h"

class MAX7320 : public I2CPort
{
public:
   MAX7320(I2CPort::IOMode_t mode, uint8_t addr, uint8_t mask) :
      I2CPort(mode, addr, mask)
   {}

  virtual uint8_t readPortData(uint8_t addr);
  virtual void    writePortData(uint8_t addr, uint8_t data);
  virtual void    writePortDDR(uint8_t addr, uint8_t ddr);
};

#endif
