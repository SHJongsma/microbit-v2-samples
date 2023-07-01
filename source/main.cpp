#include "MicroBit.h"
//#include "samples/Tests.h"

/* TO DO

### Voor de temperatuursensor ###
x Beide CRC implementaties vergelijken qua uitkomst, i.e. beide toepassen en vergelijken <== Resultaat blijkt gelijk
x Voor TH alleen eerste 4 bit beschouwen + `sign-bit' in ogenschouw nemen
x Bij foute CRC waarschuwing printen en temperatuur niet opslaan
x UpdateSample functie toevoegen
x create_fiber aan roepen met updatesample functie en dan `gewoon' oude waarde meteen teruggeven <== Lijkt niet te werken
x Expliciet checken wanneer de conversie klaar is <== Lijkt alsof het instantaan is
x Resultaat van presence pulse check daadwerkelijk gebruiken
x ROM code/ID proberen uit te lezen
x Aan de hand van uitgelezen ROM CRC berekening checken
x Spelen met timings voor hopelijk een robuuster resultaat/ Nagaan wat sommige timings `doen' op basis van documentatie
x Configuratie kunnen updaten
x Gekopieerde functies nalopen op basis van documentatie
(- Proberen een CodalComponent van de temperatuursensor te maken en dan de idlecallback implementeren) <-- Voegt niet zo veel toe

### Voor de netwerkmodule ###
- W5500 bibliotheek etc toevoegen


### Voor de uiteindelijke functionaliteit ###
- TBD





*/

#include <memory>

//#include "MicroBitSerial.h"
#include "NRF52Serial.h"
#include "MicroBitIO.h"

#include "SPI.h"

#include "include/logger.h"
#include "include/one_wire.h"
#include "include/DS18B20.h"

namespace {

MicroBit uBit;

std::shared_ptr<shj::Logger> logger = nullptr;

std::shared_ptr<shj::DS18B20> sensor; // NOTE: Unique_ptr does not seem to compile

int button_b_pressed = false;

void onButtonB(MicroBitEvent) {
  if (logger)
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

      if (logger)
        logger->debug("temperatur_test ~ showing temperature on display.");

      //int T = uBit.thermometer.getTemperature();
      double T = sensor->get_temperature();

      temp[0] = (char) ((((int) T) / 10) + 48);
      temp[1] = (char) ((((int) T) % 10) + 48);
      temp[2] = '.';
      switch (((int) (10 * T)) % 10) {
        case 9:
          temp[3] = '9';
          break;
        case 8:
          temp[3] = '8';
          break;
        case 7:
          temp[3] = '7';
          break;
        case 6:
          temp[3] = '6';
          break;
        case 5:
          temp[3] = '5';
          break;
        case 4:
          temp[3] = '4';
          break;
        case 3:
          temp[3] = '3';
          break;
        case 2:
          temp[3] = '2';
          break;
        case 1:
          temp[3] = '1';
          break;
        case 0:
        default:
          temp[3] = '0';
          break;
      }

      uBit.display.scroll({temp, 4});

      fiber_sleep(2000);

      if (button_b_pressed) {
        button_b_pressed = false;
        break;
      }
    }
  }

  // Return
  return;
}


void initialize() {


  // TO BE IMPLEMENTED: Move stuff from the first part of main here


}

double get_temperature() {

  if (!sensor && logger) {
    logger->error("::get_temperature ~ Failed to get temperature, sensor = nullptr.");
    return -999;
  }


  return 0;
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
  ::uBit.init();
  ::uBit.display.print("Start"); // will pause here a little while while showing Start

  ::uBit.serial.redirect(uBit.io.P0, uBit.io.P1); // <-- Lijkt te werken, maar dan is de logger niet te gebruiken

  // Create a logger object
  ::logger = std::make_shared<shj::Logger>(&uBit);

  //uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);

  ::logger->debug("main ~ Program started.");

  // Create a one wire interface
  shj::OneWire one_wire(::uBit.io.P2, ::logger);

  // Create a temperature sensor
  sensor = std::make_shared<shj::DS18B20>(one_wire, ::logger);

  //sensor->update_config(shj::DS18B20::resolution_t::twelve);




  // Test
  //double temp = sensor->get_temperature();

  //ManagedString tstr((int) temp);

  //uBit.display.scroll(tstr);

/*
  NRF52Pin P0(ID_PIN_P0, P0_02, PIN_CAPABILITY_AD);

  logger->debug("main ~ P0 created.");

  NRF52Pin P1(ID_PIN_P1, P0_03, PIN_CAPABILITY_AD);

  logger->debug("main ~ P1 created.");
*/
  uBit.sleep(250);

  // Werkt nog niet, misschien gebruik maken van MicroBitIO object en daar de pinnummers vandaan halen


  codal::SPI spi(uBit.io.P15, uBit.io.P14,uBit.io.P13);

  //NRF52Serial serial(uBit.io.P0, uBit.io.P1, NRF_UARTE1);


  ::temperature_test();

  uBit.sleep(250);

  //logger->debug("main ~ Serial created");

  //printMemoryAndStop();

  release_fiber(); // finished with setup, release the fibers!!
}

