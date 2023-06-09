
#include "logger.h"


#include "MicroBit.h"


namespace shj {


/*****

Idee:
- debug, info, etc. slaan berichten op in een buffer (zijn producers) en laten aan de message bus weten dat er een
 `event' is gebeurd, te weten dat er een nieuw bericht verzonden moet worden
- De consumer wacht op een event, wanneer deze gebeurt, dan haalt hij het bericht op en probeert deze
  te versturen via de seriële connectie
- Het bericht wordt vervolgens uit de buffer verwijderd

Nog uitzoeken hoe ik in de buffer bijhoudt waar welk bericht staat ==> Linked list, met log-level, bericht en pointer naar volgende
bericht.

- Op basis van het aantal keren dat op knop A gedrukt wordt het log-niveau aanpassen




*******/


void Logger::debug(const std::string &msg) {

  producer(level_t::debug, msg);

  // Return
  return;
}

void Logger::info(const std::string &msg) {

  producer(level_t::info, msg);

  // Return
  return;
}

void Logger::warn(const std::string &msg) {

  producer(level_t::warn, msg);

  // Return
  return;
}

void Logger::error(const std::string &msg) {

  producer(level_t::error, msg);

  // Return
  return;
}



void Logger::producer(const level_t level, const std::string &msg) {

  if (!m_first) {
    m_first = new LogQueue(level, msg);
  } else {

    // Counter
    int count = 0;

    // Get the final item of the queue
    LogQueue *p = m_first;
    while (p->next) {
      ++count; // Increment the counter
      p = p->next;
    }

    // If the count exceeds the maximum queue size, drop the message
    // because otherwize we might run into memory issues due to
    // LogQueue objects not being freed
    if (count >= m_max_queue)
      return;

    // Create a new LogQueue object
    p->next = new LogQueue(level, msg);
  }

  // Notify the consumer that a new event has occurred
  MicroBitEvent(DATA_ID, NEW_LOG_MESSAGE);

  // Return
  return;
}


void Logger::consumer(MicroBitEvent /*evt*/) {

  // NOTE: eventueel kan er een timestamp verkregen worden van het MicroBitEvent object

  // Call the function that does the actual processing
  process();

  // Return
  return;
}


void Logger::change_level(MicroBitEvent /*evt*/) {

  switch (m_verbose) {
    case level_t::debug:
      m_verbose = level_t::info;
      info("Logger::change_level ~ Changed to log-level: INFO.");
      break;
    case level_t::info:
      m_verbose = level_t::warn;
      break;
    case level_t::warn:
      m_verbose = level_t::error;
      break;
    case level_t::error:
      m_verbose = level_t::debug;
      info("Logger::change_level ~ Changed to log-level: DEBUG.");
      break;
  } // Closing brace for switch

  // Return
  return;
}


void Logger::process() {

  int indx = 0;

  // Check if there is an m_first
  if (!m_first) {

    // NOTE: this should not happen, therefore we send an error

    // Construct a suitable error message and have it send later on
    const std::string error("[ERROR] - Logger::process ~ There are no messages to process.");

    for (const char c : error)
      m_buffer[indx++] = c;

  } else {

    bool send = true;

    // Check if the message is sufficiently urgent
    if (m_first->level < m_verbose)
      send = false;

    if (send) {

      std::string level;
      switch (m_first->level) {
        case level_t::debug:
          level = "[DEBUG]";
          break;
        case level_t::info:
          level = "[ INFO]";
          break;
        case level_t::warn:
          level = "[ WARN]";
          break;
        case level_t::error:
          level = "[ERROR]";
          break;
      } // Closing brace for switch

      level += " - "; // Add separator

      // Copy the level label to the buffer
      for (const char c : level)
        m_buffer[indx++] = c;

      // Copy the message to the buffer
      for (const char c : m_first->msg) {
        m_buffer[indx++] = c;
        if (indx >= m_buffer_size)
          break; // Stop copying message, if it exceeds the buffer length
      }
    }

    // Remove the first item from the queue
    LogQueue *p = m_first->next;
    delete m_first;
    m_first = p;

    // Check if the message has a sufficiently high level, otherwise we return here
    if (!send)
      return;
  }

  // add newline
  m_buffer[indx] = '\r';
  ++indx;
  m_buffer[indx] = '\n';
  ++indx;
  m_buffer[indx] = '\0'; // terminate buffer

  // Here we need to send the buffer via the serial connection
  int rtn = m_mbit->serial.send((uint8_t *)m_buffer, indx);
  while (rtn == MICROBIT_SERIAL_IN_USE) {
      // try again later, wait for TX buffer empty
      fiber_wait_for_event(MICROBIT_ID_NOTIFY, MICROBIT_SERIAL_EVT_TX_EMPTY);
      rtn = m_mbit->serial.send((uint8_t *)m_buffer, indx);
  }

  // Return
  return;
}

// must call this from a fiber!!
void Logger::send_buffer() {
    int size = m_buffer_indx;
    m_buffer_indx = m_buffer_size + 2; // block any further additions until write is finished

    // add newline
    m_buffer[size] = '\r';
    ++size;
    m_buffer[size] = '\n';
    ++size;
    m_buffer[size] = '\0'; // terminate buffer

    // Send the buffer
    int rtn = m_mbit->serial.send((uint8_t *)m_buffer, size);
    while (rtn == MICROBIT_SERIAL_IN_USE) {
        // try again later, wait for TX buffer empty
        fiber_wait_for_event(MICROBIT_ID_NOTIFY, MICROBIT_SERIAL_EVT_TX_EMPTY);
        rtn = m_mbit->serial.send((uint8_t *)m_buffer, size);
    }
    buffer_init(); // clear buffer and put in leading \r\n
}

void Logger::buffer_init() {
    m_buffer[0]   = '\r';
    m_buffer[1]   = '\n';
    m_buffer[2]   = '\0'; // terminate buffer
    m_buffer_indx = 2;;
}

} // Closing brace for namespace





