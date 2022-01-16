# include "menu_entry.h"
# include <ctype.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <iostream>
# include <sstream>
# include <algorithm>

#include "esp_log.h"

#define TAG __PRETTY_FUNCTION__

using namespace std;

extern char *p_buf;
extern char *ibuf;

static bool dbg = 0;
static bool dbg1 = 0;
static char tbuf[80];
static char entry[80];

MenuEntry::KeyMap_t MenuEntry::m_keyMap;

MenuEntry::MenuEntry(const char *name, const char *key, MenuType_t type, MenuEntry *parent) :
    m_name(name),
    m_key(key),
    m_type(type),
    m_value(NULL),
    m_action(NULL),
    m_input(NULL),
    m_output(NULL),
    m_validator(NULL),
    m_parent(parent),
    m_1stChild(NULL),
    m_nextSibling(NULL)
{
//   cout << __PRETTY_FUNCTION__ << (const char*)" Constructor" << endl;
   if ( parent != NULL ) 
   {
      dbg1 = 0;
      parent->AddChildMenu(this, key);
   }
}

MenuEntry::MenuEntry(const char *name, const char *key, MenuType_t type, Variant* value, MenuEntry *parent, Validator* validator) :
    m_name(name),
    m_key(key),
    m_type(type),
    m_value(value),
    m_action(NULL),
    m_input(NULL),
    m_output(NULL),
    m_validator(validator),
    m_parent(parent),
    m_1stChild(NULL),
    m_nextSibling(NULL)
{
//   cout << __PRETTY_FUNCTION__ << (const char*)" Constructor" << endl;
   if ( parent != NULL ) 
   {
      parent->AddChildMenu(this, key);
   }
}

MenuEntry::MenuEntry(const char *name, const char *key, MenuType_t type, bool (*input)(const string), void (*output)(std::ostream& d_out), MenuEntry *parent) :
    m_name(name),
    m_key(key),
    m_type(type),
    m_value(NULL),
    m_action(NULL),
    m_input(input),
    m_output(output),
    m_validator(NULL),
    m_parent(parent),
    m_1stChild(NULL),
    m_nextSibling(NULL)
{
//   cout << __PRETTY_FUNCTION__ << (const char*)" Constructor" << endl;
//   cout << (const char*)"- m_input = " << (uint32_t)m_input << endl;
//   cout << (const char*)"- m_output = " << (uint32_t)m_output << endl;
   if ( parent != NULL ) 
   {
      parent->AddChildMenu(this, key);
   }
}

MenuEntry::MenuEntry(const char *name, const char *key, MenuType_t type, bool (*action)(std::ostream& d_out), MenuEntry *parent) :
    m_name(name),
    m_key(key),
    m_type(type),
    m_value(NULL),
    m_action(action),
    m_input(NULL),
    m_output(NULL),
    m_validator(NULL),
    m_parent(parent),
    m_1stChild(NULL),
    m_nextSibling(NULL)
{
//   cout << __PRETTY_FUNCTION__ << (const char*)" Constructor";
//   cout << (const char*)"- m_action = " << (uint32_t)m_action << endl;
   if ( parent != NULL ) 
   {
      parent->AddChildMenu(this, key);
   }
}

void MenuEntry::AddChildMenu(MenuEntry *child, const char *key)
{
   child->m_nextSibling = m_1stChild;
   if (! m_keyMap.insert(make_pair(key, child)).second)
   {
      cout << "Duplicate menu key (" << key << ") used for entry " << string(child->Name()) << endl;
      abort();
   }
   m_1stChild = child;
}

string MenuEntry::pathToString()
{
   stringstream str_path;
   MenuEntry *menu_names[64];
   int8_t menu_depth = 0;

   if ( m_parent == NULL ) 
   {
      menu_names[menu_depth] = this;
      menu_depth++;
   }
   else 
   {
      for (MenuEntry *p = this; p != NULL; p = p->m_parent ) 
      {
         if ( p->m_parent != NULL ) 
         {
            menu_names[menu_depth] = p;
            menu_depth++;
         }
      }
   }

   while (menu_depth > 0) 
   {
      str_path << menu_names[--menu_depth]->m_key;
      if ( menu_depth > 0 ) 
      {
         str_path << ".";
      }
   }
   return(str_path.str());
}

void MenuEntry::PrintPath(std::ostream& d_out)
{
   MenuEntry *menu_names[64];
   int8_t menu_depth = 0;

   if ( m_parent == NULL ) 
   {
      menu_names[menu_depth] = this;
      menu_depth++;
   }
   else 
   {
      for (MenuEntry *p = this; p != NULL; p = p->m_parent ) 
      {
         if ( p->m_parent != NULL ) 
         {
            menu_names[menu_depth] = p;
            menu_depth++;
         }
      }
   }

   while (menu_depth > 0) 
   {
      d_out << menu_names[--menu_depth]->m_name;
      if ( menu_depth > 0 ) 
      {
         d_out << (const char*)".";
      }
   }
}

#if 0
string MenuEntry::toString()
{
   stringstream str_entry;
//   str_entry << "Calculating max length" << endl;
   if ( m_type == MainMenu || m_type == SubMenu ) 
   {
      MenuEntry *p = m_1stChild;
      for (; p != NULL; p = p->m_nextSibling) 
      {
         if ( p->m_type != Action && p->m_type != MainMenu && p->m_type != SubMenu ) 
         {
            str_entry << p->toString();
         }
         if (p->m_nextSibling != NULL)
         {
            str_entry << ", ";
         }
      }
   }
   else
   {
      if ( m_type != Action && m_type != MainMenu && m_type != SubMenu)
      {
         str_entry << pathToString() << " : ";
         valueToString();
      }
   }
   return(str_entry.str());
}
#endif

bool MenuEntry::PrintEntry()
{
   bool rv = true;
//   cout << "Calculating max length" << endl;
   if ( m_type == MainMenu || m_type == SubMenu ) 
   {
      cout << endl; // Serial.print((const char*)"\r\n");
      cout << (const char*)" **** [";
      PrintPath();
      cout << (const char*)"] ****";
      cout << (const char*)"\r\n";
      int entry_len = 0;
      for (MenuEntry *p = m_1stChild; p != NULL; p = p->m_nextSibling) 
      {
//         cout << (const char*)" 0x";
//         cout << itoa((long)p, ibuf, 16);
//         cout << (const char*)", ";
         memset(tbuf, '\x00', sizeof(tbuf));
         strncpy(tbuf, p->m_name, sizeof(tbuf) - 1);
         int len = strlen(p->m_name) + p->m_key.length() + 2;
         if (dbg) cout << "entry: " << tbuf << ", len " << (int)len << ", entry_len = " << (int)entry_len << endl;
         if ( len > entry_len ) 
         {
            cout << "Found new entry_len" << endl;
            entry_len = len;
         }
      }
      cout << (const char*)"\r\n";

      MenuEntry *p = m_1stChild;
      char index[2] = "`";
      for (; rv != false && p != NULL; p = p->m_nextSibling) 
      {
         if (p->m_type != Hidden)
         {
            (*index)++;
            cout << index << ") " << p->m_key << ": " << p->m_name;
            
            if ( p->m_type != Action && p->m_type != MainMenu && p->m_type != SubMenu ) 
            {
               if (p->m_type != Hidden) 
               {
                  int targ_len = entry_len + 4 - (strlen(p->m_name) + p->m_key.length() + 2);
                  for (int x = 0; x < targ_len; x++) 
                  {
                     cout << (const char*)".";
                  }
                  cout << (const char*)" [";
                  p->PrintValue();
                  cout << (const char*)"]\r\n";
               }
            }
            else
            {
               cout << (const char*)"\r\n";
            }
         }
      }
   }
   else
   {
      cout << m_name;
      if ( m_type != Action && m_type != MainMenu && m_type != SubMenu)
      {
         cout << (const char*)" [";
         PrintValue();
         cout << (const char*)"]";
      }
   }
   return(rv);
}

string MenuEntry::valueToString()
{
   stringstream str_entry;
   switch (m_type) 
   {
      case Float:
      case Hidden:
      case Int:
      case String:
      case UInt:
      case YesNo:
         str_entry << m_value->toString();
         cout << m_value->toString();
         break;

      case TimeMin:
         {
            int val = m_value->toInt();
            int hr = val / 60;
            int min = val % 60;
            sprintf(tbuf, "%02d:%02d", hr, min);
//                        ESP_LOGI(TAG, "Showing time %02d:%02d", hr, min);
            cout << tbuf;
         }
         break;

      case Action:
      case CustomIO:
      case MainMenu:
      case SubMenu:
         break;
   }
   return(str_entry.str());
}

bool MenuEntry::PrintValue(std::ostream& d_out)
{
   char outbuf[80];
   memset(outbuf, '\x00', sizeof(outbuf));
   switch (m_type) 
   {
      case CustomIO:
         if ( m_output != NULL ) 
         {
            (*m_output)(d_out);
         }
         break;

      case Float:
      case Hidden:
      case Int:
      case UInt:
      case String:
      case YesNo:
         d_out << m_value->toString();
         break;

      case TimeMin:
         {
            int val = m_value->toInt();
            int hr = val / 60;
            int min = val % 60;
            sprintf(tbuf, "%02d:%02d", hr, min);
//            ESP_LOGI(TAG, "Showing time %02d:%02d", hr, min);
            d_out << tbuf;
         }

      case Action:
      case MainMenu:
      case SubMenu:
         break;
   }
   d_out << outbuf;
   return(true);
}

MenuEntry *MenuEntry::Execute(const char *buf, MenuEntry *first_menu, bool &change_made, ostream& d_out, bool json)
{
   MenuEntry *rv = first_menu;

   cout << "MenuEntry::Execute(): Enter!" << endl;
   memset(entry, '\x00', sizeof(entry));
   if ( strlen(buf) > 0 ) 
   {
      if ( m_type == MainMenu ||
            m_type == SubMenu ) 
      {
         char ch = 'a';
         char *cp = entry;
         while (ch >= '\x20' && ch != '?' && ch != '.' && ch != '=' ) 
         {
            if ( strlen(entry) < sizeof(entry) - 1 ) 
            {
               ch = *(buf++);
               if ( ch >= '\x20' && ch != '?' && ch != '.' && ch != '=' && ch  ) 
               {
                  *(cp++) = tolower(ch);
               }
            }
         }
         if (dbg1) cout << (const char*)"entry = '" << entry;
         if (dbg1) cout << (const char*)"'" << endl;
         if ( strlen(entry) > 0 ) 
         {
            MenuEntry *it = FindChild(entry);
            if ( it != NULL ) 
            {
//               if (dbg) cout << __PRETTY_FUNCTION__ << "found menu = '" << strcpy(tbuf, it->m_name) << "'" << endl;
//               if (dbg) cout << __PRETTY_FUNCTION__ << "ch = " << (int)ch << endl;
               if ( ch == '.' || ch == '=' ) 
               {
                  if (ch != '=' || strlen(buf) > 0)
                  {
                     rv = it->Execute(buf, first_menu, change_made, d_out, json);
                  }
                  else
                  {
                     it->setValue(Variant(""));
                  }
               }
               else if ( ch == '?')
               {
                  if (it->m_type != Hidden)
                  {
                     if (! json)
                     {
                        d_out << endl;
                        d_out << "=";
                        d_out << it->valueToString();
                        d_out << " @ ";
                        d_out << it->pathToString();
                        d_out << endl;
                     }
                     else
                     {
                        d_out << "{\"message_type\" : \"config\", ";
                        d_out << "\"" << it->pathToString() << "\" : ";
                        if (m_type == String)
                        {
                           d_out << "\"" << it->m_value->toString() << "\"}";
                        }
                        else if (m_type == TimeMin)
                        {
                           int val = m_value->toInt();
                           int hr = val / 60;
                           int min = val % 60;
                           sprintf(tbuf, "%02d:%02d", hr, min);
   //                        ESP_LOGI(TAG, "Showing time %02d:%02d", hr, min);
                           d_out << tbuf;
                        }
                        else
                        {
                           d_out << it->m_value->toString() << "}";
                        }
                     }
                  }
                  change_made = false;
               }
               else if ( isspace(ch) || ch == '\0')
               {
                  /***********************************************/
                  /*   If  we  are  at  an Action entry, do it,  */
                  /*   otherwise just return the pointer to the  */
                  /*   entry                                     */
                  /***********************************************/
                  if ( it->m_type == Action )
                  {
                     if ( it->m_action != NULL ) 
                     {
                        (*it->m_action)(d_out);
                     }
                     else 
                     {
                        cout << (const char*)"Invalid Action defined\r\n";
                     }
                  }
                  else if (it->m_type != Hidden)
                  {
                     if (dbg) cout << (const char*)"Setting rv to entry: " << strcpy(tbuf, it->m_name) << endl;
                     rv = it;
                  }
               }
               else
               {
                  cout << (const char*)"Error- Unexpected character after '";
                  cout << entry;
                  cout << (const char*)"'\r\n";
               }
            }
            else 
            {
//               cout << (int)(SP);
               cout << (const char*)"\r\nAck!!!\r\n";
//               cout << entry;
            }
         }
      }
      else 
      {
//         if (dbg) cout << __PRETTY_FUNCTION__ << "single entry, buf = " << buf;
//         if (dbg) cout << (const char*)"\r\n";
//         if (dbg) cout << __PRETTY_FUNCTION__ << "m_type = '" << (int)m_type << endl;
         switch (m_type) 
         {
            case CustomIO:
               if ( m_input != NULL ) 
               {
                  change_made = (*m_input)(string(buf));
               }
               break;

            case Action:
               change_made = (*m_action)(d_out);
               break;

            case Float:
            case Int:
            case UInt:
            case String:
               {
                  Variant val(buf);
                  bool err = false;
                  if (m_type == Float)
                  {
                     if (dbg) cout << __PRETTY_FUNCTION__ << "Calling toDouble()" << endl;
                     val.toDouble(&err);
                  }
                  else if (m_type == Int)
                  {
                     if (dbg) cout << __PRETTY_FUNCTION__ << "Calling toInt()" << endl;
                     val.toInt(&err);
                  }
                  else if (m_type == UInt)
                  {
                     int x;
                     if (dbg) cout << __PRETTY_FUNCTION__ << "Calling toUInt()" << endl;
                     if ((x = val.toUInt(&err)) < 0)
                     {
                        err = true;
                     }
                  }
                  if (! err && m_validator != NULL)
                  {
                     err = ! m_validator->validate(val);
                  }
                  if (err)
                  {
                     cout << (const char*)"\r\nAck!!!\r\n";
                     return(this);
                  }
                  change_made = true;
                  *m_value = val;
               }
               break;

            case TimeMin:
               int hour;
               int min;
               while (*buf && isspace(*buf))
                  buf++;
//               sscanf(buf, (const char*)"%d:%d", &hour, &min);
               sscanf(buf, "%d:%d", &hour, &min);
               cout << (const char*)"TimeMin Entry buf '" << buf;
               cout << (const char*)"', hour = " << (int)hour;
               cout << (const char*)", min = " << (int)min << endl;
               if (hour > 23 || hour < 0)
               {
                  cout << (const char*)"Invalid hour entered, should be 0 - 23\r\n";
               }
               else if (min > 59 || min < 0)
               {
                  cout << (const char*)"Invalid minute entered, should be 0 - 59\r\n";
               }
               else 
               {
                  int time_min = hour * 60 + min;
                  m_value->setValue(Variant(time_min));
                  change_made = true;
               }
//               if (dbg) cout << __PRETTY_FUNCTION__ << "TimeInt = " << (uint16_t)*((uint16_t*)m_value);
//               if (dbg) cout << (const char*)"'\r\n";
               break;

            case YesNo:
               while (*buf && isspace(*buf))
                  buf++;
               if ( tolower(*buf) == 'y' || tolower(*buf) == 't' ) 
               {
                  m_value->setValue(Variant("true"));
                  change_made = true;
               }
               else if ( tolower(*buf)== 'n' || tolower(*buf) == 'f' ) 
               {
                  m_value->setValue(Variant("false"));
                  change_made = true;
               }
               else 
               {
                  cout << (const char*)"Error- Unexpected value, expecting  'y' or 'n', 't' or 'f'";
               }
               break;
            case Hidden:
               break;

            default:
               cout << (const char*)"Invalid nenu type encountered!!!";
         }
         if (false && m_type != Hidden)
         {
            cout << endl;
            PrintPath(d_out);
            cout << "=";
            PrintValue(d_out);
            cout << endl;
         }
      }
   }
//   if (dbg) cout << (const char*)"Exit- Returning entry: " << strcpy(tbuf, rv->m_name) << endl;
   return(rv);
}

MenuEntry *MenuEntry::FindChild(const char *entry)
{
   MenuEntry *rv = NULL;
//   if (dbg) cout << __PRETTY_FUNCTION__ << "entry = '" << entry << "'" << endl;
   if ( strlen(entry) == 1 ) 
   {
      char ch = *entry;
      if ( isalpha(ch) ) 
      {
         if ( ch <= 'Z' ) 
         {
            ch += '\x20';
         }
      }
      uint8_t idx = *(entry) - 'a';
      rv = m_1stChild;
      for (uint8_t x = 0; rv != NULL && x < idx; x++) 
      {
         rv = rv->m_nextSibling;
      }
   }
   else 
   {
      string name;
      MenuEntry *it;
      for (it = m_1stChild; it != NULL; it = it->m_nextSibling)
      {
         name = it->m_key;
//         std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::toupper(c);});
         if (name == string(entry)) 
         {
            rv = it;
         }
      }
   }
   return(rv);
}

void MenuEntry::setValue(string key, Variant value)
{
   *m_keyMap.at(key)->m_value = value;
}

MenuEntry::KeyList_t MenuEntry::keys()
{
   KeyList_t rv;

   KeyMap_t::iterator pos;
   for (pos = m_keyMap.begin(); pos != m_keyMap.end(); ++pos)
   {
      rv.push_back(pos->first);
   }
   return(rv);
}

Variant MenuEntry::entryValue(string key)
{
//   cout << __PRETTY_FUNCTION__ << ": key = " << key << endl;
   Variant val;
   string my_val;
   if (m_keyMap.find(key) != m_keyMap.end())
   {
      MenuEntry* entry = m_keyMap.at(key);
      if (entry->m_type != MainMenu && 
          entry->m_type != MenuEntry::SubMenu &&
          entry->m_type != MenuEntry::Action &&
          entry->m_type != MenuEntry::CustomIO)
      {
         my_val = entry->m_value->toString();
   //   val.setValue(m_keyMap.at(key));
         val.setValue(my_val);
      }
   }
   return(val);
}

void MenuEntry::PrintChildren()
{
   for (MenuEntry *p = this; p != NULL; p = p->m_nextSibling ) 
   {
      if ( p->m_type == SubMenu || p->m_type == MainMenu )  
      {
         p->PrintEntry();
         p->m_1stChild->PrintChildren();
      }
   }
}

MenuEntry *MenuEntry::findEntry(std::string key)
{
   MenuEntry *rv = NULL;
   KeyMap_t::iterator pos;
   for (pos = m_keyMap.begin(); pos != m_keyMap.end(); ++pos)
   {
      if (pos->first == key)
      {
         rv = pos->second;
         break;
      }
   }
   return(rv);
}
