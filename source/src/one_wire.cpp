

#include "one_wire.h"

#include "utils.h"


namespace shj {

// Implemenation of the constructor
OneWire::OneWire(codal::Pin &pin, const std::shared_ptr<Logger> &logger) :
  m_pin(&pin),
  m_logger(logger) {

  // Nothing to be done (yet)

}


void OneWire::reset() const {
  m_pin->setDigitalValue(0); // Have bus master pull low
  sleep_us(500);             // Wait for a while (between 480 us and 960 us)
  m_pin->getDigitalValue();  // Have the resistor pull up
}

void OneWire::write_byte(uint8_t data) const {
  int _data = 0;
  for(int i = 0; i < 8; ++i) {
    _data = data & 0x01;
    data >>= 1;
    if (_data) {
      m_pin->setDigitalValue(0);
      sleep_us(2);
      m_pin->setDigitalValue(1);
      sleep_us(60);
    } else {
      m_pin->setDigitalValue(0);
      sleep_us(60);
      m_pin->setDigitalValue(1);
      sleep_us(2);
    }
    //sleep_us(2);
  }
}


uint8_t OneWire::read_bit() const {
  uint8_t data;
  m_pin->setDigitalValue(0);
  sleep_us(2);
  m_pin->setDigitalValue(1);
  sleep_us(5);
  if (m_pin->getDigitalValue()) {
    data = 1;
  } else {
    data = 0;
  }
  sleep_us(55); // Complete the read time slot of at least 60 us (and at most 120 us).

  // Return the result
  return data;
}

uint8_t OneWire::read_byte() const {
  uint8_t data = 0;
  for (int i = 0; i < 8; ++i){
    uint8_t j = read_bit();
    //uBit.serial.printf("%d",j);
    sleep_us(2); // Waarom hier nog slapen?
    data = (j << 7) | (data >> 1);
  }
   sleep_us(2);
  //uBit.serial.printf("\r\n");
  return data;
}

int OneWire::check() const {

  int rx_time = 500;

  // Wait for the sensor to pull low
  int state = 0;
  while (m_pin->getDigitalValue()) {
    ++state;
    sleep_us(1);
    if (state >= 60){
      break;
    }
  }

  // Subtract the duration of the high bit
  rx_time -= state;

  // Sensor is sending presence pulse, i.e. low bit => time it
  state = 0;
  while (!m_pin->getDigitalValue()) {
    ++state;
    sleep_us(1);
    if (state >= 60){
      break;
    }
  }

  // Update rx_time
  rx_time -= state;

  // Complete the receive window
  sleep_us(rx_time);

  // NOTE: Documentation states that presence pulse should be between 15 us and 60 us.
  // In my timings the median is 13 us. So the choice is made to have anything above
  // 11 qualify.

  // Return the result
  if (state > 11)
    return 1;
  else
    return 0;
}

#ifndef USE_ALTERNATIVE_CRC

// Function to compute the cyclic redundancy check
// From: https://github.com/paeaetech/paeae/blob/master/Libraries/ds2482/DS2482.cpp
uint8_t OneWire::compute_crc(const uint8_t *buffer, const uint8_t len) {

  uint8_t crc = 0;

  // Loop over the bytes of data
	for (uint8_t i = 0; i < len; ++i) {

		uint8_t inbyte = buffer[i];

    // Loop over the bits of the byte
		for (uint8_t j = 0; j < 8;++j) {

			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;

			inbyte >>= 1;
		}
	}

  // Return the result
	return crc;
}

#else

// Alternative implementation from: https://github.com/adafruit/Adafruit_CircuitPython_OneWire/blob/b051ceab5f1839d002c12ef29b558ab0a3efe92d/adafruit_onewire/bus.py


uint8_t OneWire::compute_crc(const uint8_t *buffer, const uint8_t len) {

  uint8_t crc = 0;

  // Loop over the bytes of data
	for (uint8_t i = 0; i < len; ++i) {

		uint8_t inbyte = buffer[i];

    crc ^= inbyte;

    // Loop over the bits of the byte
		for (uint8_t j = 0; j < 8;++j) {

      if (crc & 0x01)
				crc = (crc >> 1) ^ 0x8C;
      else
        crc >>= 1;

      crc &= 0xFF;
		}
	}

  // Return the result
	return crc;
}

#endif // USE_ALTERNATIVE_CRC




} // Closing brace for namespace
