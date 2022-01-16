#ifndef VARIANT_H
#define VARIANT_H

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class Variant
{
public:
   Variant() {};
   Variant(int v);
   Variant(unsigned int v);
   Variant(const char* v);
   Variant(const std::string v);
   Variant(double v);
   Variant(bool v);

   Variant& setValue(const Variant other)
   {
      createSemaphore();
      this->m_value = other.m_value;
      return(*this);
   }

   int            toInt(bool *err = NULL);
   unsigned int   toUInt(bool *err = NULL);
   double         toDouble(bool *err = NULL);
   bool           toBool();
   std::string    toString();
   void           createSemaphore();

   Variant& operator=(Variant& other)
   {
      createSemaphore();
      if (this == &other)
      {
         return(*this);
      }
      this->m_value = other.m_value;
      return(*this);
   }

   friend bool operator==(const Variant &v1, const Variant &v2);

private:
   std::string                m_value;
   static StaticSemaphore_t   m_varMutexBuf;
   static SemaphoreHandle_t   m_varMutex;
};

inline bool operator==(const Variant &v1, const Variant &v2)
{
   return(v1.m_value == v2.m_value);
}

#endif
