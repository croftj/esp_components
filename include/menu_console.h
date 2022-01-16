#ifndef MENU_CONSOLE_H
#define MENU_CONSOLE_H
#include <thread>
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "esp_pthread.h"
#include "linenoise/linenoise.h"
#include <cstring>
#include <iostream>
#include <sstream>
#include "menu_entry.h"

#ifndef TAG
#define TAG __PRETTY_FUNCTION__
#endif

class MenuConsole
{
public:
   MenuConsole(uart_port_t port_num);

   void setBaud(int baud) {}
   void setStopBits(uart_stop_bits_t bits) {}
   void setParity(uart_parity_t parity) {}
   void setMenu(MenuEntry *menu)
   {
      m_menu = menu;
      m_currentMenu = menu;
   }

   std::string execCommand(std::string command, bool json)
   {
      std::stringstream str_value;
      bool change_made = false;
      m_menu->Execute(command.c_str(), m_menu, change_made, str_value, json);
      if (true && change_made)
      {
         changeMade();
      }
      return(str_value.str());
   }

   void *exec(void*)
   {
      bool needPrompt = true;
      char *line_p;
      std::string line_str;
      
      while (true)
      {
         bool change_made = false;
         if (needPrompt)
         {
            std::cout << (const char*)"\n";
            needPrompt = false;
            m_currentMenu->PrintEntry();
         }
         try 
         {
            line_p = linenoise((const char*)"$: ");
            lockVars();
            if (strlen(line_p) > 0)
            {
               std::cout << "Test: " << line_p << std::endl;
               linenoiseHistoryAdd(line_p);
               m_currentMenu = m_currentMenu->Execute(line_p, m_currentMenu, change_made);
               if (true && change_made)
               {
                  changeMade();
               }
            }
            else
            {
               m_currentMenu = m_currentMenu->Parent();
            }
            releaseVars();
            linenoiseFree(line_p);
         }
         catch (...)
         {
         }
         esp_task_wdt_reset();
         needPrompt = true;
      }
   };

   MenuEntry::KeyList_t keys()
   {
      return(m_menu->keys());
   }
   virtual void changeMade() = 0;

   void lockVars()
   {
      m_varMutex = xSemaphoreCreateMutexStatic(&m_varMutexBuf);
      if (! xSemaphoreTake(m_varMutex, 1000 / portTICK_RATE_MS))
      {
         ESP_LOGW(TAG, "Semaphore blocked");
      }
   }

   void releaseVars()
   {
      xSemaphoreGive(m_varMutex);
   }

protected:
   static MenuEntry*          m_menu;
   static StaticSemaphore_t   m_varMutexBuf;
   static SemaphoreHandle_t   m_varMutex;

private:
   uart_port_t          m_portNum;
   static MenuEntry*    m_currentMenu;
};
#endif

