

#include <cstdio>
#include <cmath>




#include "codal-core/inc/driver-models/Timer.h"
#include "DS18B20.h"

#include "utils.h"

namespace {

void update_sample(void *vp_sensor) {

  const shj::DS18B20 *sensor = ((shj::DS18B20 *) vp_sensor);

  sensor->update_sample();

  // Return
  return;
}

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

  // Just return the most recent value (from cache)
  return m_last_temperature;

  // We need to obtain a new temperature from the sensor
  start();

  //fiber_sleep(750); // NOTE: Zou eigenlijk (maximaal) 750 ms moeten zijn, maar
                      // 100 us lijkt eigenlijk net zo goed te werken
  sleep_us(100); // Conversion can take up to 750 MILLIseconds
                 // NOTE, that this can be checked by listening for a one send by the sensor



  unsigned char byte_buffer[9];
  size_t count(0);
    while (true) { // NOTE: Bij maximum aantal loop afbreken, nu kans op infinite loop
    m_one_wire.reset();
    m_one_wire.check(); // Checks for precence pulse
    sleep_us(2);

    // Send ROM command over one wire
    m_one_wire.write_byte(OneWire::SKIP_ROM);

    // Moet hier nog een pauze tussen?

    // Send function command over one wire
    m_one_wire.write_byte(READ_SCRATCH);

  /*
    // Read the least signficant bit
    unsigned int TL = m_one_wire.read_byte();
    //uBit.serial.printf("TL=%d\r\n",TL);

    sleep_us(100); // Wait a while

    // Read the most significant byte
    unsigned int TH = m_one_wire.read_byte();
  */

    // Read all bytes
    for (size_t i = 0; i < 9; ++i) {
      byte_buffer[i]  = m_one_wire.read_byte();
      sleep_us(100); // Wait a while <-- Check if this is long or short enough !!!
    }

    uint8_t crc = OneWire::compute_crc(byte_buffer, 8); // Compute CRC
    if (byte_buffer[8] == crc || count > 15)
      break;
    else
      ++count;
  }

  if (count >= 15)
    m_logger->warn("DS18B20::get_temperature ~ reached maximum number of 15 tries.");

  char buffer[50];

  int nchar = sprintf(buffer, "Number of tries = %d.", (int) count);
  m_logger->debug("DS18B20::get_temperature ~ " + std::string(buffer));

  unsigned char LSB = byte_buffer[0];
  nchar = sprintf(buffer, "TL = %d.", (int) byte_buffer[0]);
  m_logger->debug("DS18B20::get_temperature ~ Read byte: " + std::string(buffer));

  unsigned char MSB = byte_buffer[1];
  nchar = sprintf(buffer, "TH = %d.", (int) byte_buffer[1]);
  m_logger->debug("DS18B20::get_temperature ~ Read byte: " + std::string(buffer));
  //uBit.serial.printf("TH=%d\r\n",TH);

  // Also get the CRC, which is byte 9 on index 8
  unsigned char CRC = byte_buffer[8];
  nchar = sprintf(buffer, "CRC = %d.", (int) CRC);
  m_logger->debug("DS18B20::get_temperature ~ Read byte: " + std::string(buffer));

  // Also compute the CRC, based on the data in the buffer
  uint8_t crc = OneWire::compute_crc(byte_buffer, 8);
  nchar = sprintf(buffer, "crc = %d.", (int) crc);
  m_logger->debug("DS18B20::get_temperature ~ Read byte: " + std::string(buffer));

  nchar = sprintf(buffer, "(%d, %d, %d, %d, %d, %d, %d, %d, %d).", (int) byte_buffer[0], (int) byte_buffer[1],
    byte_buffer[2], byte_buffer[3], byte_buffer[4], byte_buffer[5], byte_buffer[6], byte_buffer[7], byte_buffer[8]);
  m_logger->info("Got bytes: " + std::string(buffer));

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

