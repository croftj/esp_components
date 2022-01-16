#include "OneWireNg.h"
#include <cstring>
#include <string>
#include <sstream>

class OneWireDevice 
{
public:
   OneWireDevice()
   {
      memset(addr, 0, 8);
      iop = 0;
   }

   void initialize(OneWire *p, uint8_t *a) 
   {
      memcpy(addr, a, 8);
      iop = p;
   }

   std::string address()
   {
      std::stringstream addr_stream;

      for(int i = 0; i < 8; i++)
      {
         addr_stream << std::hex << (int)addr[i];
         if (i < 7)
         {
            addr_stream << ":";
         }
      }
      return(addr_stream.str());
   }

   OneWire  *iop;
   uint8_t  addr[8];

private:
protected:
};
