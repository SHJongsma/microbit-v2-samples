

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

double compute_temperature(const unsigned char LSB, const unsigned char MSB, const unsigned char config) {

  double conversion = 0.0625;
  int shift = 0;
  switch (config >> 4) {
    case (0):
      conversion  = 0.5;
      shift       = 3;
      break;
    case (1):
      conversion  = 0.25;
      shift       = 2;
      break;
    case (2):
      conversion  = 0.125;
      shift       = 1;
      break;
    case (3):
    default:
      conversion  = 0.0625;
      shift       = 0;
      break;
  }

  unsigned int temp = MSB & 0x07;
  temp = (temp << 8) | LSB;

  if (shift)
    temp >>= shift;

  double result;
  // Check if temperature is negative
  if ((MSB & 0xF8) == 0xF8) {
		temp = (temp ^ 0xFFFF) + 1; // NOTE: Negative temperatures might not be computed correctly
		result = temp * (-conversion);
	}else{
		result = temp * conversion; // Resolutie nog instellen
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

  // Read and store the ID
  read_ROM();

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



void DS18B20::start(const unsigned char function_code) const {

  int present = 0;
  while (!present) {
    m_one_wire.reset();  // Reset sensor
    present = m_one_wire.check(); // Wait for presence pulse
  }

  sleep_us(2);

  // Send ROM command
  m_one_wire.write_byte(OneWire::SKIP_ROM);

  // Send function command
  m_one_wire.write_byte(function_code);
}


void DS18B20::update_sample() const {

  const size_t max_ticks = 12097;

  // We need to obtain a new temperature from the sensor
  start();

  // Wait for the conversion to be done
  // NOTE: One tick amounts to 62 us
  // A conversion of a 12 bit temperature takes at most
  // 750 ms, which equals 12097 ticks. If this number of
  // ticks is exceeded, we break the loop
  size_t tick = 0;
  while (true) {
    ++tick;

    if (m_one_wire.read_bit() || (tick > max_ticks))
      break;
  }

  if (tick >= max_ticks) {
    m_logger->warn("DS18B20::update_sample ~ Maximum conversion time timed out.");
  }

  char buffer[60];
  int nchar = sprintf(buffer, "Conversion took = %d us.", tick * 62);
  m_logger->debug("DS18B20::update_sample ~ " + std::string(buffer));

  unsigned char byte_buffer[9];
  read_bytes(9, byte_buffer, [this]() {

    // Send ROM command over one wire
    m_one_wire.write_byte(OneWire::SKIP_ROM);

    // Moet hier nog een pauze tussen?

    // Send function command over one wire
    m_one_wire.write_byte(READ_SCRATCH);

  });

/*
  // NOTE: Store the ROM code
  nchar = sprintf(buffer, "(%d, %d, %d, %d, %d, %d, %d, %d, crc = %d).", (int) byte_buffer[0], (int) byte_buffer[1],
  byte_buffer[2], byte_buffer[3], byte_buffer[4], byte_buffer[5], byte_buffer[6], byte_buffer[7], byte_buffer[8]);
  m_logger->info("Got bytes: " + std::string(buffer));
*/

  // Get the least significant byte
  const unsigned char LSB = byte_buffer[0];

  // Get the most significant byte
  const unsigned char MSB = byte_buffer[1];

  // Get the configuration
  const unsigned char config = byte_buffer[4];

  // Store the temperature in cache
  m_last_temperature = ::compute_temperature(LSB, MSB, config);

  // Update the time of the last read
  m_last_read = codal::system_timer_current_time();

  // Return
  return;
}

void DS18B20::read_bytes(const size_t buffer_size, unsigned char *byte_buffer,
  std::function<void()> read_function, const size_t max_tries) const {

  size_t count(0);
  while (true) {
    m_one_wire.reset();

    if (!m_one_wire.check()) { // Checks for presence pulse
      m_logger->info("DS18B20::read_bytes ~ Presence pulse returned 0.");
      continue;
    }

    sleep_us(2);

    // Call the specific read function
    read_function();

    // Read all bytes
    for (size_t i = 0; i < buffer_size; ++i)
      byte_buffer[i]  = m_one_wire.read_byte();

    uint8_t crc = OneWire::compute_crc(byte_buffer, buffer_size - 1); // Compute CRC over the first 'buffer_size - 1' bytes
    if (byte_buffer[buffer_size - 1] == crc || count > max_tries)
      break;
    else
      ++count;
  }

  if (count >= max_tries)
    m_logger->warn("DS18B20::read_bytes ~ reached maximum number of tries.");

  // Return
  return;
}

void DS18B20::read_ROM() {

  unsigned char byte_buffer[8];

  read_bytes(8, byte_buffer, [this]() { m_one_wire.write_byte(OneWire::READ_ROM); });

/*
  uint8_t crc = OneWire::compute_crc(byte_buffer, 7); // Compute CRC over the first seven bytes

  // NOTE: Store the ROM code
  char buffer[60];
  int nchar = sprintf(buffer, "(%d, %d, %d, %d, %d, %d, %d, %d, crc = %d).", (int) byte_buffer[0], (int) byte_buffer[1],
  byte_buffer[2], byte_buffer[3], byte_buffer[4], byte_buffer[5], byte_buffer[6], byte_buffer[7], crc);
  m_logger->info("Got bytes: " + std::string(buffer));
*/

  // Store the ID (skip family code and CRC)
  for (size_t i = 0; i < 6; ++i)
    m_ID[i] = byte_buffer[i+1];

  // Return
  return;
}

std::tuple<unsigned char, unsigned char, unsigned char> DS18B20::read_config() const {

  unsigned char byte_buffer[9];

  read_bytes(9, byte_buffer, [this]() {

    // Send ROM command over one wire
    m_one_wire.write_byte(OneWire::SKIP_ROM);

    // Send function command over one wire
    m_one_wire.write_byte(READ_SCRATCH);

  });

/*
  uint8_t crc = OneWire::compute_crc(byte_buffer, 8); // Compute CRC over the first seven bytes

  // NOTE: Store the ROM code
  char buffer[60];
  int nchar = sprintf(buffer, "(%d, %d, %d, %d, %d, %d, %d, %d, crc = %d).", (int) byte_buffer[0], (int) byte_buffer[1],
  byte_buffer[2], byte_buffer[3], byte_buffer[4], byte_buffer[5], byte_buffer[6], byte_buffer[7], crc);
  m_logger->info("Got bytes: " + std::string(buffer));
*/

  // Return the config
  return {byte_buffer[2], byte_buffer[3], byte_buffer[4]};
}

void DS18B20::update_config(const resolution_t res, const unsigned char TH, const unsigned char TL) {

  unsigned char byte_buffer[3] = {TH, TL, 0};

  switch (res) {
    case resolution_t::nine:
      byte_buffer[2] = 0x1F;
      break;
    case resolution_t::ten:
      byte_buffer[2] = 0x3F;
      break;
    case resolution_t::eleven:
      byte_buffer[2] = 0x5F;
      break;
    case resolution_t::twelve:
      byte_buffer[2] = 0x7F;
      break;
  }

  // Store the value to compare later on
  const unsigned char compare = byte_buffer[2];

  // Call start function
  start(WRITE_SCRATCH);

  // Send the bytes
  for (size_t i = 0; i < 3; ++i)
    m_one_wire.write_byte(byte_buffer[i]);

  // Next, read the config to verify the result
  unsigned char th;
  unsigned char tl;
  unsigned char read_res;
  std::tie(th, tl, read_res) = read_config();

  // Check if temperatures are correctly stored
  if ((TH == th) && (TL == tl) && (compare == read_res)) {
    // Call start function
    start(COPY_SCRATCH);
  }

  // Return
  return;
}



} // Closing brace for namespace

