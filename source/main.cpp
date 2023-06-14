#include "MicroBit.h"
//#include "samples/Tests.h"

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

/*

int main() {
    ::uBit.init();

    //out_of_box_experience();
    ::temperature_test();

    microbit_panic( 999 );
}

*/

//#include "MicroBit.h"
#define UNUSED(x) (void)(x)

const int DATA_ID = MICROBIT_ID_NOTIFY+1; // last defined eventId is MICROBIT_ID_NOTIFY==1023 in MicroBitComponent.h
const int NEW_DATA = 1;

// struct for the data
struct DATA_STRUCT {
    int count;
    char c;
};

struct DATA_STRUCT data; // the global to update and access between fibers

const int debugBufferSize = 62;
char debugBuffer[debugBufferSize+2]; // allow 1 for terminating null and 1 for newline
int debugBufferIdx = 0; // next location to write to leave first char for \n
int dumpInterval = 5000;

void debugBufferInit() {
    debugBuffer[0] = '\n';
    debugBuffer[1] = '\0'; // terminate buffer
    debugBufferIdx = 1;;
}

void debugChar(char c) {
    if (debugBufferIdx>=debugBufferSize) {
        return; // buffer full
    }
    debugBuffer[debugBufferIdx] = c;
    debugBufferIdx++;
    debugBuffer[debugBufferIdx] = '\0'; // terminate buffer
}

// must call this from a fiber!!
void sendDebugBuffer() {
    int size = debugBufferIdx;
    debugBufferIdx = debugBufferSize+1; // block any further additions until write is finished
    // add newline
    debugBuffer[size] = '\n';
    size++;
    debugBuffer[size] = '\0'; // terminate buffer
    int rtn = uBit.serial.send((uint8_t *)debugBuffer,size);
    while (rtn == MICROBIT_SERIAL_IN_USE) {
        // try again later, wait for TX buffer empty
        fiber_wait_for_event(MICROBIT_ID_NOTIFY, MICROBIT_SERIAL_EVT_TX_EMPTY);
        rtn = uBit.serial.send((uint8_t *)debugBuffer,size);
    }
    debugBufferInit(); // clear buffer and put in leading \n
}

void producer() {
    while(1) { // loop for ever
        MicroBitEvent(DATA_ID,NEW_DATA);
        uBit.sleep(20); // wait a short while
        data.c = (char)('A'+data.count); // next char in sequence
        debugChar(data.c);
        data.count++;
        if (data.count>=26) {
            data.count = 0;
        }
    }
}

void consumer(MicroBitEvent /*evt*/) {
    //UNUSED(evt); // keep Netbeans from complaining about unreferenced evt
    struct DATA_STRUCT localData;
    localData.c = data.c;
    localData.count = data.count;
    uBit.serial.sendChar(localData.c);
    uBit.display.printChar(localData.c,1000);
    uBit.serial.send(localData.count);
    uBit.display.print(localData.count);
}

void debugOut(void *intervalPtr) {
    int interval = *((int*)intervalPtr);
    while(1) {
        uBit.sleep(interval); // every 5 sec
        sendDebugBuffer();
    }
}





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

/*
int main() {
  uBit.init();
  uBit.display.print("Start"); // will pause here a little while while showing Start

  // Create a logger object
  logger = new shj::Logger(&uBit);

  //uBit.messageBus.listen(MICROBIT_ID_BUTTON_B, MICROBIT_BUTTON_EVT_CLICK, onButtonB);

  logger->debug("main ~ Program started.");

  ::temperature_test();

  //printMemoryAndStop();

  release_fiber(); // finished with setup, release the fibers!!
}
*/


#if CONFIG_ENABLED(DEVICE_BLE)


namespace shj {

MicroBitUARTService *uart;

// we use events abd the 'connected' variable to keep track of the status of the Bluetooth connection
static void onConnected(MicroBitEvent)
{
    uBit.display.print("C");
}

static void onDisconnected(MicroBitEvent)
{
    uBit.display.print("D");
}

static void onDelim(MicroBitEvent)
{
    ManagedString r = uart->readUntil("\r\n");
    uart->send(r);
}

}

int main() {

    // Configuration Tips
    //
    // An example codal.json is provided in the root directory: codal.ble.json
    // Rename codal.ble.json to codal.json to use this BLE sample
    //
    // codal.json contains various Bluetooth related properties some of which are explained here:
    //
    // "DEVICE_BLE": 1                     Determines whether BLE is enabled
    // "MICROBIT_BLE_ENABLED" : 1          Determines whether BLE is enabled
    // "MICROBIT_BLE_PAIRING_MODE": 1      Determines whether Pairing Mode is enabled
    // "MICROBIT_BLE_DFU_SERVICE": 1       Determines whether the Nordic DFU Service is enabled
    // "MICROBIT_BLE_DEVICE_INFORMATION_SERVICE": 1 Determines whether the DIS is enabled
    // "MICROBIT_BLE_EVENT_SERVICE" : 1,   Determines whether the Event Service is enabled
    // "MICROBIT_BLE_PARTIAL_FLASHING" : 0 Determines whether Partial Flashing is enabled (Needs MakeCode/Python)
    // "MICROBIT_BLE_SECURITY_LEVEL": "SECURITY_MODE_ENCRYPTION_WITH_MITM" Determines security mode
    //
    // Options for MICROBIT_BLE_SECURITY_LEVEL are:
    // SECURITY_MODE_ENCRYPTION_WITH_MITM, enables pairing with a passcode
    // SECURITY_MODE_ENCRYPTION_NO_MITM, enables pairing without a passcode
    // SECURITY_MODE_ENCRYPTION_OPEN_LINK, pairing is no required
    //

    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_CONNECTED, shj::onConnected);
    uBit.messageBus.listen(MICROBIT_ID_BLE, MICROBIT_BLE_EVT_DISCONNECTED, shj::onDisconnected);

    uBit.messageBus.listen(MICROBIT_ID_BLE_UART, MICROBIT_UART_S_EVT_DELIM_MATCH, shj::onDelim);

    new MicroBitAccelerometerService(*uBit.ble, uBit.accelerometer);
    new MicroBitButtonService(*uBit.ble);
    new MicroBitIOPinService(*uBit.ble, uBit.io);
    new MicroBitLEDService(*uBit.ble, uBit.display);
    new MicroBitMagnetometerService(*uBit.ble, uBit.compass);
    new MicroBitTemperatureService(*uBit.ble, uBit.thermometer);

    shj::uart = new MicroBitUARTService(*uBit.ble, 32, 32);
    shj::uart->eventOn("\r\n");

    // A cunning code to indicate during start-up the particular Bluetooth configuration in the build
    //
    // SERVICE CODES
    // A: Accelerometer Service
    // B: Button Service
    // D: Device Information Service
    // E: Event Service
    // F: DFU Service
    // I: IO Pin Service
    // L: LED Service
    // M: Magnetometer Service
    // T: Temperature Service
    // U: UART Service
    //

    // P: PASSKEY
    // J: Just Works
    // N: No Pairing Required

    // Services/Pairing Config/Power Level
    uBit.display.scroll("BLE ABDILMTU/P");

    if ( !uBit.compass.isCalibrated())
        uBit.compass.calibrate();

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();

  // Return 0 to indicate success
  return 0;
}

#endif




