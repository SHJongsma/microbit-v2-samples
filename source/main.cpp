#include "MicroBit.h"
//#include "samples/Tests.h"

namespace {

MicroBit uBit;

int button_a_pressed = false;
int button_b_pressed = false;

void onButtonA(MicroBitEvent) {
  button_a_pressed = true;
}

void onButtonB(MicroBitEvent) {
  button_b_pressed = true;
}


void temperature_test() {

  char temp[4];

  uBit.messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, onButtonA);
  uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);

  while (true) {

    // Wait for button A press.
    while(!button_a_pressed) {
        //uBit.display.print("A");
        uBit.sleep(250);
        if(button_a_pressed) {
           button_a_pressed = false;
           break;
        }
    }

    button_b_pressed = false;

    // Display temperature
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

      if (button_b_pressed) {
        button_b_pressed = false;
        break;
      }
    }
  }

  // Return
  return;
}

} // Namespace

int main() {
    ::uBit.init();

    //out_of_box_experience();
    ::temperature_test();

    microbit_panic( 999 );
}

