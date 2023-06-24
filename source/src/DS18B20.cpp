

#include <cstdio>
#include <cmath>




#include "codal-core/inc/driver-models/Timer.h"
#include "DS18B20.h"

#include "utils.h"

namespace {

void update_sample(void *vp_sensor) {

  const shj::DS18B20 *sensor = ((shj::DS18B20 *) vp_sensor);

 // sensor->update_sample(); // <== Function to be implemented

  // Return
  return;
}

std::string double_to_string(const double &x) {

  double val(x);

  // Determine the number of digits
  int digits = 1;
  if (x >= 1)
    digits = static_cast<int>(std::log10(x)) + 1;

  // Compute the base
  double base = 1;
  int tmp = digits - 1;
  while (tmp > 0) {
    base *= 10;
    --tmp;
  }

  // Compose the string
  std::string result;
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

  // We need to obtain a new temperature from the sensor
  start();
  sleep_us(100);
  m_one_wire.reset();
  m_one_wire.check();
  sleep_us(2);

  // Send ROM command over one wire
  m_one_wire.write_byte(OneWire::SKIP_ROM);

  // Send function command over one wire
  m_one_wire.write_byte(READ_SCRATCH);

  // Read the information
  unsigned int TL = m_one_wire.read_byte();
  //uBit.serial.printf("TL=%d\r\n",TL);

  char buffer[50];
  int nchar = sprintf(buffer, "TL = %d.", TL);
  m_logger->debug("DS18B20::get_temperature ~ Read byte: " + std::string(buffer));

  sleep_us(100);
  unsigned int TH = m_one_wire.read_byte();

  nchar = sprintf(buffer, "TH = %d.", TH);
  m_logger->debug("DS18B20::get_temperature ~ Read byte: " + std::string(buffer));
  //uBit.serial.printf("TH=%d\r\n",TH);

  unsigned int temp = TH;
  temp = (temp << 8) + TL;

  double result;
  // Wat gebeurt hier? Negatieve temperatuur ogenschijnlijk
  if ((temp & 0xf800) == 0xf800) {
		temp = (~temp) + 1;
		result = temp * (-0.0625);
	}else{
		result = temp * 0.0625; // Resolutie nog instellen
	}

  m_logger->debug("DS18B20::get_temperature ~ Got result: " + ::double_to_string(result) + " C.");
  // Don't forget to update the cache values:

  // Update the time of the last read
  m_last_read = codal::system_timer_current_time();

  // Update the value of the last read
  m_last_read = result;

  // Return the result
  return result;
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



} // Closing brace for namespace

