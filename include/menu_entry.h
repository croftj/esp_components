#ifndef MENU_ENTRY_H
#define MENU_ENTRY_H

#include <map>
#include <stddef.h>
#include <stdint.h>
#include <string>
#include <iostream>
#include <vector>

#include "variant.h"
#include "validator.h"

class MenuEntry 
{
public:
   typedef std::map<std::string, MenuEntry*> KeyMap_t;
   typedef std::vector<std::string> KeyList_t;
   typedef enum
   {
      STRING,
      NUMBER,
      BOOL,
      MENU,
      HIDDEN,
      SPECIAL
   } MenuClass_t;

   typedef enum
   {
      Action,
      CustomIO,
      Float,
      Int,
      String,
      TimeMin,
      UInt,
      YesNo,
      Hidden,
      MainMenu,
      SubMenu
   } MenuType_t;

   typedef enum
   {
      NoOptions      = 0,
      Invisible      = 1 << 0,
      NoPermanence   = 1 << 2
   } Options_t;

   MenuEntry(const char *name, const char *key, MenuType_t type, MenuEntry *parent = NULL);
   MenuEntry(const char *name, const char *key, MenuType_t type, Variant *value, MenuEntry *parent, Validator* validator = NULL, int options = NoOptions);
   MenuEntry(const char *name, const char *key, MenuType_t type, bool (*input)(const std::string), void (*output)(std::ostream& d_out), MenuEntry *parent);
   MenuEntry(const char *name, const char *key, MenuType_t type, bool (*action)(std::ostream& d_out), MenuEntry *parent);

   void PrintChildren();
   bool PrintJsonEntry(std::ostream& d_out);
   void PrintPath(std::ostream& d_out = std::cout);
   bool PrintEntry();
   bool PrintValue(std::ostream& d_out = std::cout);
   static Variant entryValue(std::string key);
   static MenuEntry::KeyList_t keys();
   static MenuEntry *findEntry(std::string key);
   static void printEntries(std::ostream& d_out);
   std::string toString();
   std::string valueToString();
   std::string pathToString();
   const char *parseEntryName(const char* buf, char* name_buf, unsigned int len);
   MenuEntry *Execute(const char *buf, MenuEntry *first_menu, bool &change_made, std::ostream& d_out = std::cout, bool json = false);
   void setValue(std::string key, Variant value);

   void setValue(Variant value)
   {
      *m_value = value;
   }

   std::string key()
   {
      return(m_key);
   }

   MenuEntry *Parent()
   {
      if ( m_parent == NULL ) 
      {
         return(this);
      }
      else 
      {
         return(m_parent);
      }
   }

   char *Name()
   {
      return((char*)m_name);
   }

   MenuType_t type()
   {
      return(m_type);
   }

   MenuClass_t entryClass()
   {
      return(m_class);
   }

   int options()
   {
      return(m_options);
   }

private:
   void              AddChildMenu(MenuEntry *child, const char *key);
   MenuEntry*        FindChild(const char *entry);

   const char*       m_name;
   std::string       m_key;
   MenuType_t        m_type;
   MenuClass_t       m_class;
   Variant*          m_value;
   bool              (*m_action)(std::ostream& d_out);
   bool              (*m_input)(const std::string);
   void              (*m_output)(std::ostream& d_out);
   Validator*        m_validator;
   Options_t         m_options;
   MenuEntry*        m_parent;
   MenuEntry*        m_1stChild;
   MenuEntry*        m_nextSibling;
   static KeyMap_t   m_keyMap;
};

#endif
