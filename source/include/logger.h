#ifndef LOGGER
#define LOGGER

#include <string>

// Forward declaration
//class MicroBit;

#include "MicroBit.h"

namespace shj {


class Logger;

} // Closing brace for namespace shj

namespace {

struct Combine {


  MicroBit *mbit;
  int id;
  int value;
  shj::Logger *logger;
  void (shj::Logger::* logger_fn)(MicroBitEvent);
  void (shj::Logger::* button_fn)(MicroBitEvent);


};

// Forward declaration
void create_consumer(void *combined);


} // Closing brace for anonymous namespace

namespace shj {

class Logger {

private:

  const int DATA_ID = MICROBIT_ID_NOTIFY + 1; // last defined eventId is MICROBIT_ID_NOTIFY==1023 in MicroBitComponent.h
  const int NEW_LOG_MESSAGE = 1;

public:

  // Some enum class stuff
  // from: https://stackoverflow.com/a/15451002

  enum class level_t {debug, info, warn, error};

  // Constructor
  Logger(MicroBit *m) :
    m_mbit(m) {

    // Initialize the buffer
    //buffer_init(); // OBSOLETE?

    m_combine = {m_mbit, DATA_ID, NEW_LOG_MESSAGE, this, &Logger::consumer, &Logger::change_level};

    m_mbit->display.print("A");

    // Create the consumer
    create_fiber(::create_consumer, (void *) &m_combine);
    //m_mbit->messageBus.listen(DATA_ID, NEW_LOG_MESSAGE, this, &Logger::consumer);

    m_mbit->display.print("B");
  };


  void debug(const std::string &msg);
  void info(const std::string &msg);
  void warn(const std::string &msg);
  void error(const std::string &msg);

private:

  class LogQueue {

    public:

    LogQueue(const level_t l, const std::string m) :
      level(l),
      msg(m) {

      // Nothing to be done

    }

    level_t level;
    std::string msg;
    LogQueue *next = nullptr;
  };


  void process();

  // Is called by the log message function, e.g. debug(...)
  void producer(const level_t level, const std::string &msg);

  // Sends the serial data if the log-level is sufficiently high, otherwise the message is discarded
  void consumer(MicroBitEvent /*evt*/);

  void change_level(MicroBitEvent /*evt*/);

  const std::string m_line_end = "\r\n";
  static const int m_buffer_size = 80;
  static const int m_max_queue   = 10;
  int m_buffer_indx              = 0; // next location to write to leave first char for \n
  level_t m_verbose              = level_t::debug;
  char m_buffer[m_buffer_size + 3]; // allow 1 for terminating null and 1 for newline and carrige return

  MicroBit *m_mbit = nullptr;

  LogQueue *m_first = nullptr;

  ::Combine m_combine;

}; // Closing brace for class definintion


} // Closing brace for namespace

namespace {


void create_consumer(void *combined) {

  const ::Combine *c = ((::Combine *) combined);

  c->mbit->messageBus.listen(c->id, c->value, c->logger, c->logger_fn);
  c->mbit->messageBus.listen(MICROBIT_ID_BUTTON_A, MICROBIT_BUTTON_EVT_CLICK, c->logger, c->button_fn);
}

}

#endif // LOGGER
