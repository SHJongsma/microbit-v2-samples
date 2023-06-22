
#include "codal-core/inc/driver-models/Timer.h"
#include "DS18B20.h"

#include "utils.h"


namespace shj {


// NOTE: Also use the logger !!!

// Implemenation of the constructor
DS18B20::DS18B20(codal::Pin &pin, const std::shared_ptr<Logger> &logger) :
  m_pin(&pin),
  m_logger(logger) {

  // Nothing to be done (yet)

}


double DS18B20::get_temperature() const {

  // Check if the last time the temperature was read is recent
  size_t now = codal::system_timer_current_time();
  if (now - m_last_read < m_refresh_rate) {
    m_logger->debug("DS18B20::get_temperature ~ Returning cached temperature.");
    // Return the cached temperature
    return m_last_temperature;
  }


  // NOTE: Even nadenken hoe het bij een overflow van de klok gaat (iedere 1.6 maand)

  m_logger->debug("DS18B20::get_temperature ~ Obtaining new temperature from sensor.");

  // We need to obtain a new temperature from the sensor
  start();
  sleep_us(100);
  reset();
  check();
  sleep_us(2);
  write_byte(0xCC);
  write_byte(0xBE);

  unsigned int TL = read_byte();
  //uBit.serial.printf("TL=%d\r\n",TL);
  sleep_us(100);
  unsigned int TH = read_byte();
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

  // Don't forget to update the cache values:

  // Update the time of the last read
  m_last_read = codal::system_timer_current_time();

  // Update the value of the last read
  m_last_read = result;

  // Return the result
  return result;
}



void DS18B20::reset() const {
  m_pin->setDigitalValue(0); // Set pin to 0
  sleep_us(750);             // Wait for a while
  m_pin->setDigitalValue(1); // Set pin to 1
  sleep_us(15);              // Wait a short while
}

void DS18B20::start() const {
  reset();
  check();
  sleep_us(2);
  write_byte(0xCC);
  write_byte(0x44);
}

void DS18B20::write_byte(uint8_t data) const {
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

int DS18B20::check() const {

  int state = 0;
  while (m_pin->getDigitalValue()) {
    ++state;
    sleep_us(1);
    if (state >= 200){
      break;
    }
  }

  if(state >= 200)
    return 1;
  else
    state = 0;

  while(!m_pin->getDigitalValue()){
    ++state;
    sleep_us(1);
    if (state >= 240){
      break;
    }
  }

  if(state >= 240)
    return 1;

  return 0;
}

uint8_t DS18B20::read_bit() const {
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
  sleep_us(60); // Waarom hier nog slapen?

  // Return the result
  return data;
}

uint8_t DS18B20::read_byte() const {
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



} // Closing brace for namespace

