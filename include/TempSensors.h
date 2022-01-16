#ifndef TEMPSENSORS_H
#define TEMPSENSORS_H

# include "OneWireNg.h"
# include "OneWireDevice.h"
# include "cJSON.h"

#include <string>
#include <vector>

#define NUM_SENSORS     8
#define NUM_TEMP_PORTS  1

class TempSensors 
{
public:
   typedef enum 
   {
      CURRENT  = 0,
      MINIMUM,
      MAXIMUM,
      AVERAGE
   } SensorReading_t;

   TempSensors();
   void           initialize(int sensor_port);
   int            discover();
   void           startCycle();
   void           readNextSensor();
   float          read(uint8_t sensor, SensorReading_t reading = CURRENT, bool celsius = false);
   void           readSensors();
   bool           validSensor(uint8_t sensor);
   uint8_t*       deviceAddress(uint8_t device);
   void           dumpSensors();
   std::vector<std::string> allDevices();
   std::string    toJson(uint8_t sensor, time_t timestamp, std::string board_id, bool celsius = false);

   uint8_t  count()
   {
      return(m_deviceCount);
   }

   void     clearTotals()
   {
      for (uint8_t x = 0; x < NUM_SENSORS; x++) 
      {
         minimum[x] = +1000.0;
         maximum[x] = -1000.0;
         average[x] = 0;
         sum[x] = 0;
         totals_count[x] = 0;
      }
   }

protected:
   float    readSensor(OneWireDevice *device, bool *err = NULL);
   void     startCycle(OneWireDevice *device);
   void     printAddr(uint8_t* addr);

private:
   OneWire*       m_temp_ports[NUM_TEMP_PORTS+2];
   OneWireDevice  devices[NUM_SENSORS];
   float          temperatures[NUM_SENSORS];
   uint8_t        m_lastSensorCycled;
   uint8_t        m_deviceCount;
   float          minimum[NUM_SENSORS];
   float          maximum[NUM_SENSORS];
   float          average[NUM_SENSORS];
   float          sum[NUM_SENSORS];
   uint16_t       totals_count[NUM_SENSORS];
};

#endif
