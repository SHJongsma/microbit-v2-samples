#ifndef ONE_WIRE_HH
#define ONE_WIRE_HH

#include <memory>

#include "logger.h"

#include "codal-core/inc/driver-models/Pin.h"

namespace shj {

class OneWire {

public:

  // Constructor
  OneWire(codal::Pin &pin, const std::shared_ptr<Logger> &logger);

  void reset() const;
  int check() const;

  void write_byte(uint8_t byte) const;

  uint8_t read_bit() const ;
  uint8_t read_byte() const;

  uint8_t get_current_value() const ;

  static uint8_t compute_crc(const uint8_t *buffer, const uint8_t len);

  static const unsigned char READ_ROM   = 0x33;
  static const unsigned char SEARCH_ROM = 0xF0;
  static const unsigned char MATCH_ROM  = 0x55;
  static const unsigned char SKIP_ROM   = 0xCC;

private:

  // Private member variables
  codal::Pin *m_pin = nullptr;
  std::shared_ptr<Logger> m_logger;



}; // Closing brace for class definition


} // Closing brace for namespace


#endif // ONE_WIRE_HH

