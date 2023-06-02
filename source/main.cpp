#include "MicroBit.h"
//#include "samples/Tests.h"


//#include <sstream>

MicroBit uBit;

void temperature_test() {

  char temp[4];



  while(1) {

    int T = uBit.thermometer.getTemperature();

    temp[0] = (char) (((T/4) / 10) + 48);
    temp[1] = (char) (((T/4) % 10) + 48);
    temp[2] = '.';
    switch (T % 4) {
      case 3:
        temp[3] = '8';
        break;
      case 2:
        temp[3] = '5';
        break;
      case 1:
        temp[3] = '3';
        break;
      case 0:
      default:
        temp[3] = '0';
        break;
    }

    uBit.display.scroll({temp, 4});
  }

}
/*
void temperature_test() {

    std::ostringstream ss;

    while(1) {
        //DMESG("TEMPERATURE: %d", uBit.thermometer.getTemperature());

        int32_t T = uBit.thermometer.getTemperature();
        ss << T/4;
        ss << '.';
        switch (T % 4) {
          case 3:
            ss << '8';
            break;
          case 2:
            ss << '5';
            break;
          case 1:
            ss << '3';
            break;
          case 0:
          default:
            ss << '0';
            break;
        }
        //uBit.display.scroll(uBit.thermometer.getTemperature());
        uBit.display.scroll(ss.str().c_str());
    }
}
*/

int main() {
    uBit.init();

    //out_of_box_experience();
    temperature_test();

    microbit_panic( 999 );
}

