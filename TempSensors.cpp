# include "TempSensors.h"
# include "OneWireNg_ArduinoESP32.h"
//# include "OneWireNg_ArduinoESP8266.h"
# include "IOPort.h"
# include "json11.hpp"
# include "driver/gpio.h"
#include "esp_log.h"

#include <string.h>
#include <thread>
#include <sstream>

using namespace std;
using namespace json11;

//#define  DBG
//#define  DBG1
//#define  DBG2
//#define  DBG3

//OneWire t_sens1(17);
//OneWire t_sens2(18);
//OneWire t_sens3(19);
//OneWire t_sens4(20);

//OneWire t_port1(4);
//OneWire t_port2(5);
//OneWire t_port3(6);

#define TAG __PRETTY_FUNCTION__

TempSensors::TempSensors() : 
   m_lastSensorCycled(0)
{
}

void TempSensors::initialize(int sensor_port)
{
   ESP_LOGI(TAG, "Sensor port %d", sensor_port);
   m_temp_ports[0] = new OneWireNg_ArduinoESP32(sensor_port, true);
//   m_temp_ports[0] = new OneWireNg_ArduinoESP32(GPIO_NUM_15, true);
//   m_temp_ports[0] = new OneWireNg_ArduinoESP32(GPIO_NUM_23, true);
//   m_temp_ports[1] = &t_port2;
//   m_temp_ports[2] = &t_port3;
//   m_temp_ports[3] = &t_port4;

//   cout << "t_init NUM_TEMP_PORTS = " << NUM_TEMP_PORTS << endl;
   discover();
   ESP_LOGI(TAG, "Discovered devices:");
   for (uint8_t x = 0; x < NUM_SENSORS; x++) 
   {
      if ( devices[x].iop != NULL ) 
      {
         char tbuf[9];
         memset(tbuf, '\0', sizeof(tbuf));
         memcpy(tbuf, devices[x].addr, 8);
         cout << "dev[ " << (int)x + 1 << "]- addr: ";
         printAddr(devices[x].addr);
         cout << endl;
      }
      else 
      {
         break;
      }
   }
   if ( m_deviceCount > 0 ) 
   {
      startCycle(&devices[0]);
   }
   clearTotals();
}

float TempSensors::read(uint8_t sensor, SensorReading_t reading, bool celsius)
{
   float rv = 0.0;

   char type[16] = "";

   switch (reading) 
   {
      case MINIMUM:
         rv = minimum[sensor];
         strcpy(type, (" Minimum "));
         break;

      case MAXIMUM:
         rv = maximum[sensor];
         strcpy(type, (const char*)(" Maximum "));
         break;

      case AVERAGE:
         rv = average[sensor];
         strcpy(type, (const char*)(" Average "));
         break;

      case CURRENT:
      default:
         rv = temperatures[sensor];
         strcpy(type, (const char*)(" Current "));
         break;
   }
   ESP_LOGI(TAG, "Raw temperature %s, temp[%d]: %f", type, sensor, rv);

   if ( ! celsius ) 
   {
      rv = (rv * (9.0/5.0)) + 32.0;
   }
   ESP_LOGI(TAG, "Request %s, temp[%d]: %f", type, sensor, rv);
   return(rv);
}

void TempSensors::readNextSensor()
{
   bool err;

   OneWireDevice *device = &devices[m_lastSensorCycled];
   if ( device->iop != NULL ) 
   {
# ifdef DBG1
      cout << __PRETTY_FUNCTION__ << "t_read pointer = " << (long)device->iop << ", addr = ";
      printAddr(device->addr);
      cout << endl;
# endif
      float t = readSensor(device, &err);

      /***********************************************/
      /*   If   we   have   no   errors   keep  the  */
      /*   temperature.  It  looks like the sensors  */
      /*   will  return  185.000 with a good crc is  */
      /*   they  are unhappy about something, maybe  */
      /*   it  is  from  the  senor  getting  a bad  */
      /*   command from the noise.                   */
      /***********************************************/
      if ( ! err) 
      {
         if ( t < 84.0) 
         {
            totals_count[m_lastSensorCycled]++;
            temperatures[m_lastSensorCycled] = t;
            if ( minimum[m_lastSensorCycled] > temperatures[m_lastSensorCycled] ) 
            {
               minimum[m_lastSensorCycled] = temperatures[m_lastSensorCycled];
            }
            if ( maximum[m_lastSensorCycled] < temperatures[m_lastSensorCycled] ) 
            {
               maximum[m_lastSensorCycled] = temperatures[m_lastSensorCycled];
            }
            sum[m_lastSensorCycled] += temperatures[m_lastSensorCycled];
            average[m_lastSensorCycled] = sum[m_lastSensorCycled] / totals_count[m_lastSensorCycled];
            ESP_LOGI(TAG, "Saving temp(%f) from sensor %d, min: %f, max: %f, sum: %f, avg: %f"
                  , t
                  , m_lastSensorCycled
                  , minimum[m_lastSensorCycled]
                  , maximum[m_lastSensorCycled]
                  , sum[m_lastSensorCycled]
                  , average[m_lastSensorCycled]
                  );
         }
//         else 
//         {
//            cout << "Invalid temp (" << t << ") from sensor " << (int)m_lastSensorCycled << " Skipping" << endl;
//         }
      }

      // Otherwise, skip this temp.
//      else 
//      {
//         cout << "crc err from sensor " << (int)m_lastSensorCycled << " Skipping" << endl;
//      }
   }
   m_lastSensorCycled++;
   if ( devices[m_lastSensorCycled].iop  == NULL ) 
   {
      m_lastSensorCycled = 0;
   }
//   cout << "Starting cycle for sensor " << (int)m_lastSensorCycled << endl;
   startCycle(&devices[m_lastSensorCycled]);
}

void TempSensors::readSensors()
{
   for (uint8_t x = 0; x < NUM_SENSORS; x++) 
   {
      if ( devices[x].iop != 0 ) 
      {
//         delay(200);
         /***********************************************/
         /*   Save  the  minimum,  maximum and average  */
         /*   value for each sensor                     */
         /***********************************************/
         totals_count[x]++;
         temperatures[x] = readSensor(&devices[x]);
         if ( minimum[x] > temperatures[x] ) 
         {
            minimum[x] = temperatures[x];
         }
         if ( maximum[x] < temperatures[x] ) 
         {
            maximum[x] = temperatures[x];
         }
         sum[x] += temperatures[x];
         average[x] = sum[x] / totals_count[x];
      }
   }
}

uint8_t* TempSensors::deviceAddress(uint8_t device)
{
   uint8_t* rv;
   device = device % NUM_SENSORS;
   rv = devices[device].addr;
   return(rv);
}

std::string TempSensors::toJson(uint8_t sensor, time_t timestamp, std::string device_id, bool celsius)
{
   uint8_t *addr;
   /******************************************************/
   /* start by converting the device address to a string */
   /******************************************************/
   std::stringstream addr_string;
   addr = devices[sensor].addr;
   for (uint8_t x = 0; x < 8; x++)
   {
      addr_string << hex << (int)addr[x];
      if (x < 7)
      {
         addr_string << ":";
      }
   }
   
   ESP_LOGI(TAG, "addr: %s, min %f, max: %f, sum: %f, avg: %f", addr_string.str().c_str()
                                                              , minimum[sensor]
                                                              , maximum[sensor]
                                                              , sum[sensor]
                                                              , average[sensor]
                                                              );
//   std::cout << "::toJson(): addr: " << addr_string.str();
//   cout << ", minimum " << minimum[sensor];
//   cout << ", maximum " << maximum[sensor];
//   cout << ", sum " << sum[sensor];
//   cout << ", average " << maximum[sensor] << endl;
   json11::Json js = Json::object {
      { "sensor",      addr_string.str() },
      { "temperature", (celsius) ? temperatures[sensor] : (temperatures[sensor] * (9.0/5.0)) + 32.0 },
      { "maximum",     (celsius) ? maximum[sensor] : (maximum[sensor] * (9.0/5.0)) + 32.0 },
      { "minimum",     (celsius) ? minimum[sensor] : (minimum[sensor] * (9.0/5.0)) + 32.0 },
      { "average",     (celsius) ? average[sensor] : (average[sensor] * (9.0/5.0)) + 32.0 },
      { "timestamp",   (int)timestamp },
      { "device",      device_id }
   };
   return(js.dump());
}

float TempSensors::readSensor(OneWireDevice *device, bool *err)
{
   uint8_t data[12];

   if ( device == 0 || device->iop == 0) 
   {
      return(0.0);
   }
   OneWire* ts = device->iop;

   ts->reset();
   ts->addressSingle(device->addr);    
   ts->writeByte(0xBE);           // Read Scratchpad

# ifdef DBG
   cout << "rs.P=" << dec << (int) present << " ";
# endif

   for ( uint8_t i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ts->readByte();
# ifdef DBG
      cout << hex << (int)data[i] << " ";
# endif
   }
//   ts->reset();           // Read Scratchpad

# ifdef DBG
   cout << " rs.CRC=" << hex << (int)OneWire::crc8( data, 8) << dec << endl;
# endif

   if ( OneWire::crc8(data, 8) != data[8] ) 
   {
      *err = true;
   }
   else 
   {
      *err = false;
   }
   uint8_t msb = data[1];
   uint8_t lsb = data[0];
   int16_t value = (msb << 8) + lsb;
   float temp = value * 0.0625;

# ifdef DBG
   cout << hex << "msb = 0x" <<  (int)msb << ", lsb = 0x" <<  (int)lsb << ", value = 0x" <<  (int)value << dec << endl;
   cout << "msb: " << (uint16_t)msb << endl;
   cout << "lsb: " << (uint16_t)lsb << endl;
   cout << "val: " << value << endl;
# endif
# ifdef DBG1
   cout << "temp: " << temp << endl;
# endif
   return(temp);
}

int TempSensors::discover()
{
   uint8_t device_cnt = 0;
   uint8_t port_cnt = 0;

   uint8_t addr[8];

   for (port_cnt = 0; device_cnt < NUM_SENSORS && port_cnt < NUM_TEMP_PORTS; port_cnt++) 
   {
# ifdef DBG1
      cout << "discovering port " << (int)port_cnt << endl;
# endif
      OneWire *ts = m_temp_ports[port_cnt];
      ts->searchReset();
      if ( ts != 0 ) 
      {
         OneWireNg::ErrorCode ec;
         do
         {
            ec = ts->search(addr);
# ifdef DBG1
            cout << "sensor found port_cnt: " << (int)port_cnt << endl;
            cout << "sc.R=";
            for( i = 0; i < 8; i++) {
               cout << hex << (int)addr[i];
               cout << " ";
            }
            cout << dec << endl;
# endif

            if ( OneWire::crc8(addr, 7) != addr[7]) {
               continue;
            }
            
            if ( addr[0] != 0x28) {
               continue;
            }
            else 
            {
               devices[device_cnt].initialize(ts, addr);
               device_cnt++;
            }
         } while (ec == OneWireNg::EC_MORE);
         ts->searchReset();
      }
   }
   m_deviceCount = device_cnt;
# ifdef DBG1
   cout << "Device count = " << (int)device_cnt << endl; 
#endif
   return(device_cnt);
}

void TempSensors::startCycle()
{
//   cout << __PRETTY_FUNCTION__  << endl;
   for (uint8_t x = 0; x < NUM_SENSORS; x++) 
   {
      if ( devices[x].iop != 0 ) 
      {
# ifdef DBG1
         cout << "t_start x = " << (int)x << ", pointer = " << (long)devices[x].iop << " ";
         printAddr(devices[x].addr);
         cout << endl;
#endif
         startCycle(&devices[x]);
         this_thread::sleep_for(std::chrono::milliseconds(1000));
      }
//      cout << endl;
   }
}

void TempSensors::startCycle(OneWireDevice *device)
{
   OneWire *ts = device->iop;
   ts->reset();
   ts->addressSingle(device->addr);
//   ts->writeByte(0x44,1);         // start conversion, with parasite power on at the end
   ts->writeByte(0x44);
}


bool TempSensors::validSensor(uint8_t sensor)
{
   return(devices[sensor % NUM_SENSORS].iop != 0);
}

void TempSensors::printAddr(uint8_t* addr)
{
   cout << "sc.R=";
   for( uint8_t i = 0; i < 8; i++) {
      cout << hex << (int)addr[i];
      cout << " ";
   }
   cout << dec;
}

void TempSensors::dumpSensors()
{
   return;
   for (uint8_t x = 0; x < NUM_SENSORS; x++) 
   {
      if ( devices[x].iop != NULL ) 
      {
         cout << (const char*)("sensor [ ") << (int)x + 1;
         cout << (const char*)("]- addr: ");
         printAddr(devices[x].addr);
         cout << (const char*)(", temperature: ") << read(x + 1, CURRENT);
         cout << (const char*)(", minimum: ")     << read(x + 1, MINIMUM);
         cout << (const char*)(", maximum: ")     << read(x + 1, MAXIMUM);
         cout << (const char*)(", average: ")     << read(x + 1, AVERAGE);
         cout << endl;
      }
      else 
      {
         break;
      }
   }
}

vector<string> TempSensors::allDevices()
{
   vector<string> rv;    
   for (uint8_t x = 0; x < NUM_SENSORS; x++) 
   {
      if ( devices[x].iop != NULL ) 
      {
         rv.push_back(devices[x].address());
      }
      else 
      {
         break;
      }
   }
   return(rv);
}
