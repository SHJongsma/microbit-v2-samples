
#include "codal-core/inc/driver-models/Timer.h"
#include "DS18B20.h"

#include "utils.h"


namespace shj {


// NOTE: Also use the logger !!!

// Implemenation of the constructor
DS18B20::DS18B20(codal::Pin &pin) :
  m_pin(&pin) {

  // Nothing to be done (yet)

}


double DS18B20::get_temperature() {

  double temp;

  // Check if the last time the temperature was read is recent
  size_t now = codal::system_timer_current_time();
  if (now - m_last_read < m_refresh_rate) {
    // Return the cached temperature
    return m_last_temperature;
  }

  // We need to obtain a new temperature from the sensor
  // TO BE IMPLEMENTED

  // NOTE: Even nadenken hoe het bij een overflow van de klok gaat (iedere 1.6 maand)

  // NOTE II: Don't forget to update the cache values:
  //uint32_t m_last_read;                 // Time of last read in ms
  //double m_last_temperature;            // Last temperature recorded



  // Return the result
  return temp;
}

/*
void DS18B20Rest(void){
  pin->setDigitalValue(0);
  sleep_us(750);
  pin->setDigitalValue(1);
  sleep_us(15);
}

void DS18B20WiteByte(uint8_t data){
  int _data=0,i;
  for(i=0;i<8;i++){
    _data=data&0x01;
    data>>=1;
    if(_data){
      pin->setDigitalValue(0);
      sleep_us(2);
      pin->setDigitalValue(1);
      sleep_us(60);
    }else{
      pin->setDigitalValue(0);
      sleep_us(60);
      pin->setDigitalValue(1);
      sleep_us(2);
    }
    //sleep_us(2);
  }
}

int DS18B20ReadBit(void){
  int data;
  pin->setDigitalValue(0);
  sleep_us(2);
  pin->setDigitalValue(1);
  sleep_us(5);
  if(pin->getDigitalValue()){
      data = 1;
  }else data = 0;
  sleep_us(60);
  return data;
}

int DS18B20ReadByte(void){
  int i,j,data=0;
  for(i=0;i<8;i++){
    j = DS18B20ReadBit();
    //uBit.serial.printf("%d",j);
    sleep_us(2);
    data = (j<<7)|(data>>1);
  }
   sleep_us(2);
  //uBit.serial.printf("\r\n");
  return data;
}

int DS18B20Check(void){
    int state=0;
    while (pin->getDigitalValue())
    {
      state++;
      sleep_us(1);
      if(state>=200){
        break;
      }
    }
    if(state>=200)return 1;
    else state = 0;
    while(!pin->getDigitalValue()){
      state++;
      sleep_us(1);
      if(state>=240){
        break;
      }
    }
    if(state>=240)return 1;
    return 0;
}

void DS18B20Start(void){
  DS18B20Rest();
  DS18B20Check();
  sleep_us(2);
  DS18B20WiteByte(0xcc);
  DS18B20WiteByte(0x44);
}

float DS18B20GetTemperture(void){
  int temp;
  int TH,TL;
  float value;
  DS18B20Start();
  sleep_us(100);
  DS18B20Rest();
  DS18B20Check();
  sleep_us(2);
  DS18B20WiteByte(0xCC);
  DS18B20WiteByte(0xBE);

  TL = DS18B20ReadByte();
  //uBit.serial.printf("TL=%d\r\n",TL);
  sleep_us(100);
  TH = DS18B20ReadByte();
  //uBit.serial.printf("TH=%d\r\n",TH);
  temp=TH;
  temp=(temp<<8)+TL;

  if((temp&0xf800)==0xf800){
		temp=(~temp)+1;
		value=temp*(-0.0625);
	}else{
		value=temp*0.0625;
	}
  return value;
}





*/

} // Closing brace for namespace

