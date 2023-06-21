
#include "Timer.h"

#include "utils.h"


namespace shj {

// Function to sleep for a number of micro seconds
void sleep_us(size_t us){
  size_t lasttime = codal::system_timer_current_time_us();
  size_t nowtime  = codal::system_timer_current_time_us();
  while ((nowtime - lasttime) < us){
    nowtime = codal::system_timer_current_time_us();
  }

  // Return
  return;
}

} // Closing brace for namespace

