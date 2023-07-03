#ifndef DS18B20_HH
#define DS18B20_HH





// From:  https://github.com/DFRobot/pxt-ds18b20

// Aansluiten:
// http://www.suppertime.co.uk/blogmywiki/2019/04/microbit-ds18b20-temperature-sensor/


//#include "pxt.h"



// MicroBitPin *pin = &uBit.io.P13;

#include <memory>
#include <functional>
#include <tuple>

#include "logger.h"
#include "one_wire.h"

#include "codal-core/inc/driver-models/Pin.h"

namespace shj {

class DS18B20 {

public:

  enum class resolution_t {nine, ten, eleven, twelve};

  // Constructor
  DS18B20(const OneWire &one_wire, const std::shared_ptr<Logger> &logger);

  double get_temperature() const;

  void update_config(const resolution_t res, const unsigned char TH = 75, const unsigned char TL = 70);

private:
  // Private member functions
  void read_ROM();
  void update_sample() const;

  std::tuple<unsigned char, unsigned char, unsigned char> read_config() const;

  int check() const; // Check for a presence pulse
  void start(const unsigned char function_code = CONVERT) const;

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
  static const unsigned char COPY_SCRATCH =  0x48;

}; // Closing brace for class definition


} // Closing brace for namespace

#endif // DS18B20_HH
