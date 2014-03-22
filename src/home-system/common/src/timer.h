#ifndef TIMER_H
#define	TIMER_H

#include "ios_wrapper.h"

namespace home_system
{

class timer
{
public:
  timer();
  ~timer();
  timer(const timer&) = delete;
  timer& operator=(const timer&) = delete;
  
  void set_from_now(unsigned int duration, std::function<void()> handler);
  bool is_set();
  void cancel();
  
private:
  
  ios_wrapper ios_;
  boost::asio::deadline_timer dt_;
  bool set_;
};

}

#endif	/* TIMER_H */

