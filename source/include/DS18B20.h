#ifndef DS18B20_HH
#define DS18B20_HH





// From:  https://github.com/DFRobot/pxt-ds18b20

// Aansluiten:
// http://www.suppertime.co.uk/blogmywiki/2019/04/microbit-ds18b20-temperature-sensor/


//#include "pxt.h"



// MicroBitPin *pin = &uBit.io.P13;

#include <memory>
#include <functional>

#include "logger.h"
#include "one_wire.h"

#include "codal-core/inc/driver-models/Pin.h"

namespace shj {

class DS18B20 {

public:
  // Constructor
  DS18B20(const OneWire &one_wire, const std::shared_ptr<Logger> &logger);


  double get_temperature() const;

  void update_sample() const;


private:
  // Private member functions
  void read_ROM();

  int check() const; // Check for a presence pulse
  void start() const;

  void read_bytes(const size_t buffer_size, unsigned char *byte_buffer,
    std::function<void()> read_function, const size_t max_tries = 15) const;

  unsigned char m_ID[6];

  // Private member variables
  const OneWire &m_one_wire;
  std::shared_ptr<Logger> m_logger;

  // Variables for caching the temperature
  const uint32_t m_refresh_rate = 1000; // ms
  mutable uint32_t m_last_read;                 // Time of last read in ms
  mutable double m_last_temperature;            // Last temperature recorded

  static const unsigned char CONVERT       = 0x44;
  static const unsigned char READ_SCRATCH  = 0xBE;
  static const unsigned char WRITE_SCRATCH = 0x4E;

}; // Closing brace for class definition





/*
  //% OBSOLETE ?
int Temperature(int p) {

    switch(p){
      case 0: pin = &uBit.io.P0; break;
      case 1: pin = &uBit.io.P1; break;
      case 2: pin = &uBit.io.P2; break;
      case 5: pin = &uBit.io.P5; break;
      case 8: pin = &uBit.io.P8; break;
      case 11: pin = &uBit.io.P11; break;
      case 12: pin = &uBit.io.P12; break;
      case 13: pin = &uBit.io.P13; break;
      case 14: pin = &uBit.io.P14; break;
      case 15: pin = &uBit.io.P15; break;
      case 16: pin = &uBit.io.P16; break;
      default: pin = &uBit.io.P0;
    }
    float data1,data2,data3;
    data1=DS18B20GetTemperture();
    data2=DS18B20GetTemperture();
    if((data2-data1)>2){
        return data1*10;
    }else{
        return data2*10;
    }

  }

*/

} // Closing brace for namespace

#endif // DS18B20_HH
