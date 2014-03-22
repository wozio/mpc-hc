#ifndef YAMICONTAINER_H
#define	YAMICONTAINER_H

#include <yami4-cpp/yami.h>

#define YC home_system::yami_container::instance()
#define AGENT YC.agent()

namespace home_system
{

class yami_container
{
public:
  static yami_container& instance()
  {
    static yami_container i;
    return i;
  }
  
  class event_callback_impl : public yami::event_callback
  {
    void incoming_connection_open(const char * target);
    void outgoing_connection_open(const char * target);
    void connection_closed(const char * target);
    void connection_error(const char * target);
  } ec_;
  
  yami::agent& agent()
  {
    return agent_;
  }
  
  const std::string& endpoint()
  {
    return endpoint_;
  }

  void operator()(int ec, const char* desc);

private:
  yami_container();
  
  yami::agent agent_;
  std::string endpoint_;
};

}

#endif	/* YAMICONTAINER_H */

