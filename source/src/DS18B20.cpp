

#include <cstdio>
#include <cmath>




#include "codal-core/inc/driver-models/Timer.h"
#include "DS18B20.h"

#include "utils.h"

namespace {

/*
void update_sample(void *vp_sensor) {

  // NOTE: For some reason calling update_sample from a different fiber does not work, it fails
  // on the DS18B20::start function.

  const shj::DS18B20 *sensor = ((shj::DS18B20 *) vp_sensor);

  sensor->update_sample();

  // Return
  return;
}
*/

std::string double_to_string(const double &x) {

  double val(x);

  std::string result;
  if (val < 0) {
    result += '-';
    val *= -1;
  }

  // Determine the number of digits
  int digits = 1;
  if (val >= 1)
    digits = static_cast<int>(std::log10(val)) + 1;

  // Compute the base
  double base = 1;
  int tmp = digits - 1;
  while (tmp > 0) {
    base *= 10;
    --tmp;
  }

  // Compose the string
  while (base >= 1) {
    int D = val / base;
    result += static_cast<char>(D + 48);

    val -= base * D;
    base /= 10;
  }

  result += '.';

  // Add the first decimal digit
  val *= 10;

  // Note, that some rounding down has been observe here
  result += static_cast<char>(static_cast<int>(val) + 48);

  // Return the result
  return result;
}

double compute_temperature(const unsigned char LSB, const unsigned char MSB) { // <== Maybe add parameter for resolution

  unsigned int temp = MSB & 0x07;
  temp = (temp << 8) | LSB;

  double result;
  // Check if temperature is negative
  if ((MSB & 0xF8) == 0xF8) {
		temp = (temp ^ 0xFFFF) + 1; // NOTE: Negative temperatures might not be computed correctly
		result = temp * (-0.0625);
	}else{
		result = temp * 0.0625; // Resolutie nog instellen
	}

  // Return the result
  return result;
}

} // Closing brace for anonymous namespace

namespace shj {



// Implemenation of the constructor
DS18B20::DS18B20(const OneWire &one_wire, const std::shared_ptr<Logger> &logger) :
  m_one_wire(one_wire),
  m_logger(logger) {

  // Nothing to be done (yet)

}


double DS18B20::get_temperature() const {

  // Check if the last time the temperature was read is recent
  size_t now = codal::system_timer_current_time();

  // Prevent problems with overflow (every 1.6 months)
  if (now < m_last_read)
    m_last_read = 0; // Reset m_last_read to zero

  if (now - m_last_read < m_refresh_rate) {
    m_logger->debug("DS18B20::get_temperature ~ Returning cached temperature.");
    // Return the cached temperature
    return m_last_temperature;
  }

  m_logger->debug("DS18B20::get_temperature ~ Obtaining new temperature from sensor.");

  // Update the sample in a different fiber
  //create_fiber(::update_sample, (void *) this);

  // Update sample function from here
  update_sample();

  // Just return the just obtained value (stored in cache)
  return m_last_temperature;
}



void DS18B20::start() const {
  m_one_wire.reset();
  m_one_wire.check();
  sleep_us(2);

  // Send ROM command
  m_one_wire.write_byte(OneWire::SKIP_ROM);

  // Send function command
  m_one_wire.write_byte(CONVERT);
}


void DS18B20::update_sample() const {

  const size_t max_tries = 15;

  // We need to obtain a new temperature from the sensor
  start();

  //fiber_sleep(750); // NOTE: Zou eigenlijk (maximaal) 750 ms moeten zijn, maar
                      // 100 us lijkt eigenlijk net zo goed te werken
  sleep_us(100); // Conversion can take up to 750 MILLIseconds
                 // NOTE, that this can be checked by listening for a one send by the sensor

  unsigned char byte_buffer[9];
  size_t count(0);
  while (true) {
    m_one_wire.reset();
    m_one_wire.check(); // Checks for presence pulse
    sleep_us(2);

    // Send ROM command over one wire
    m_one_wire.write_byte(OneWire::SKIP_ROM);

    // Moet hier nog een pauze tussen?

    // Send function command over one wire
    m_one_wire.write_byte(READ_SCRATCH);

    // Read all bytes
    for (size_t i = 0; i < 9; ++i) {
      byte_buffer[i]  = m_one_wire.read_byte();
      sleep_us(100); // Wait a while <-- Check if this is long or short enough !!!
    }

    uint8_t crc = OneWire::compute_crc(byte_buffer, 8); // Compute CRC over the first eight bytes
    if (byte_buffer[8] == crc || count > max_tries)
      break;
    else
      ++count;
  }

  if (count >= 15)
    m_logger->warn("DS18B20::update_sample ~ reached maximum number of 15 tries.");

  // Get the least significant byte
  const unsigned char LSB = byte_buffer[0];

  // Get the most significant byte
  const unsigned char MSB = byte_buffer[1];

  // Store the temperature in cache
  m_last_temperature = ::compute_temperature(LSB, MSB);

  // Update the time of the last read
  m_last_read = codal::system_timer_current_time();

  // Return
  return;
}

} // Closing brace for namespace

