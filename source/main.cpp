#include "MicroBit.h"
//#include "samples/Tests.h"


//#include "MicroBitSerial.h"
#include "NRF52Serial.h"
#include "MicroBitIO.h"

#include "include/logger.h"

namespace {

MicroBit uBit;

shj::Logger *logger;

int button_b_pressed = false;

void onButtonB(MicroBitEvent) {
  logger->info("Button B pressed.");
  button_b_pressed = true;
}


void temperature_test() {

  char temp[4];

  uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);

  while (true) {

    // Wait for button A press.
    while (!button_b_pressed) {
        uBit.sleep(250);
        if (button_b_pressed) {
            break;
        }
    }

    button_b_pressed = false;

    // Display temperature
    while(1) {

      logger->debug("temperatur_test ~ showing temperature on display.");

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

/****

Idee:

Wanneer de temperatuur van het afvoerwater binnen 5 graden van de temperatuur van de BBC Micro temperatuursensor zit, dan wordt
de kleur gelijk aan de kleurtemperatuur van 3000 K. Bij een groter verschil (het afvoer water wordt aangenomen warmer te zijn
dan de BBC Micro) verloopt de kleur van Signaalrood (#155,36,35) --- voor de maximale watertemperatuur --- naar koningsblauw (#65, 105, 225) --- voor de actuele BBC Micro temperatuur (Nog een geschikt verloop uitzoeken van rood naar blauw).

Zaken waar vermoedelijk aandacht aan moet worden besteed:
- Bij het aanzetten van het licht (opnieuw) verbinding maken met de LED lamp (en de actuele kleur instellen)
- Uitlezen van de temperatuursensor



****/


void printMemoryAndStop() {
    int blockSize = 64; int i = 1;
    uBit.serial.printf("Checking memory with blocksize %d char ...\r\n", blockSize);
    while (true) {
        char *p = (char *) malloc(blockSize);
        if (p == NULL) break; // this actually never happens, get a panic(20) outOfMemory first
        uBit.serial.printf("%d + %d/16 K\r\n", i/16, i%16);
        ++i;
    }
}


int main() {
  uBit.init();
  uBit.display.print("Start"); // will pause here a little while while showing Start


  // Create a logger object
  logger = new shj::Logger(&uBit);

  //uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);

  logger->debug("main ~ Program started.");


/*
  NRF52Pin P0(ID_PIN_P0, P0_02, PIN_CAPABILITY_AD);

  logger->debug("main ~ P0 created.");

  NRF52Pin P1(ID_PIN_P1, P0_03, PIN_CAPABILITY_AD);

  logger->debug("main ~ P1 created.");
*/
  uBit.sleep(250);

  // Werkt nog niet, misschien gebruik maken van MicroBitIO object en daar de pinnummers vandaan halen
  //NRF52Serial serial(uBit.io.P0, uBit.io.P1); // Gaat nog steeds mis


  //logger->debug("main ~ Serial created");

  ::temperature_test();

  //printMemoryAndStop();

  release_fiber(); // finished with setup, release the fibers!!
}

