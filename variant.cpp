#include "variant.h"
#include <iostream>

StaticSemaphore_t       Variant::m_varMutexBuf;
SemaphoreHandle_t       Variant::m_varMutex;

void Variant::createSemaphore()
{
   static bool have_semaphore = false;
   if (! have_semaphore)
   {
      m_varMutex = xSemaphoreCreateMutexStatic(&m_varMutexBuf);
      xSemaphoreGive(m_varMutex);
      have_semaphore = true;
   }
}

Variant::Variant(int v)
{
   char buf[80];
   snprintf(buf, sizeof(buf), "%d", v);
   createSemaphore();
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      m_value = buf;
      xSemaphoreGive(m_varMutex);
   }
}

Variant::Variant(unsigned int v)
{
   char buf[80];
   snprintf(buf, sizeof(buf), "%u", v);
   createSemaphore();
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      m_value = buf;
      xSemaphoreGive(m_varMutex);
   }
}

Variant::Variant(double v)
{
   char buf[80];
   snprintf(buf, sizeof(buf), "%f", v);
   createSemaphore();
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      m_value = buf;
      xSemaphoreGive(m_varMutex);
   }
}

Variant::Variant(const std::string v)
{
   createSemaphore();
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      m_value = v;
      xSemaphoreGive(m_varMutex);
   }
}

Variant::Variant(const char* v)
{
   createSemaphore();
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      m_value = std::string(v);
      xSemaphoreGive(m_varMutex);
   }
}

Variant::Variant(bool f)
{
   createSemaphore();
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      if (f)
         m_value = std::string("true");
      else
         m_value = std::string("false");
      xSemaphoreGive(m_varMutex);
   }
}

int Variant::toInt(bool *err)
{
   int rv = 0;
   const char *scp;
   char *ecp;
   
   if (err != NULL)
   {
      *err = false;
   }
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      scp = m_value.c_str();
      rv = strtol(scp, &ecp,0);
   //   std::cout << "Variant::toInt(): rv = " << rv << ", scp = " << (long)scp << ", ecp = " << (long)ecp << std::endl;
      if (err != NULL)
      {
         if (*scp == '\0' || scp == ecp || *ecp != '\0')
         {
            *err = true;
            rv = 0;
         }
      }
      xSemaphoreGive(m_varMutex);
   }
   return(rv);
}

unsigned int Variant::toUInt(bool *err)
{
   unsigned int rv = 0;
   const char *scp;
   char *ecp;
   
   if (err != NULL)
   {
      *err = false;
   }
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      scp = m_value.c_str();
      rv = strtoul(scp, &ecp, 0);
   //   std::cout << "Variant::toUInt(): rv = " << rv << ", scp = " << (long)scp << ", ecp = " << (long)ecp << std::endl;
      if (err != NULL)
      {
         if (*scp == '\0' || scp == ecp || *ecp != '\0')
         {
            *err = true;
            rv = 0;
         }
      }
      xSemaphoreGive(m_varMutex);
   }
   return(rv);
}

double Variant::toDouble(bool *err)
{
   double rv = 0.0;
   const char *scp;
   char *ecp;
   
   if (err != NULL)
   {
      *err = false;
   }
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      scp = m_value.c_str();
      rv = strtod(scp, &ecp);
   //   std::cout << "Variant::toDouble(): rv = " << rv << ", scp = " << (long)scp << ", ecp = " << (long)ecp << std::endl;
      if (err != NULL)
      {
         if (*scp == '\0' || scp == ecp || *ecp != '\0')
         {
            *err = true;
            rv = 0;
         }
      }
      xSemaphoreGive(m_varMutex);
   }
   return(rv);
}

std::string Variant::toString()
{
   std::string rv;
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      rv = m_value;
      xSemaphoreGive(m_varMutex);
   }
   return(rv);
}

bool Variant::toBool()
{
   bool rv = false;
   if (xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
   {
      if (m_value == "true")
         rv = true;
      xSemaphoreGive(m_varMutex);
   }
   return(rv);
}

