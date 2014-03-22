#include "timer.h"

namespace home_system
{

timer::timer()
: dt_(ios_.io_service()),
  set_(false)
{
}

timer::~timer()
{
  cancel();
}

void timer::set_from_now(unsigned int duration, std::function<void()> handler)
{
  cancel();
  dt_.expires_from_now(boost::posix_time::milliseconds(duration));
  dt_.async_wait([handler, this] (const boost::system::error_code& error){
    set_ = false;
    if (error)
    {
      return;
    }
    handler();
  });
  set_ = true;
}

bool timer::is_set()
{
  return set_;
}

void timer::cancel()
{
  if (set_)
  {
    set_ = false;
    dt_.cancel();
  }
}

}
