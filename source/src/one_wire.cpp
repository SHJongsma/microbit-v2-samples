

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
  m_pin->setDigitalValue(0); // Set pin to 0
  sleep_us(750);             // Wait for a while
  m_pin->setDigitalValue(1); // Set pin to 1
  sleep_us(15);              // Wait a short while
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
  sleep_us(60); // Waarom hier nog slapen?

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






} // Closing brace for namespace
