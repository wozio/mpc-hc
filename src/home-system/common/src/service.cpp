#include "service.h"
#include "mcs.h"
#include "discovery.h"
#include "logger.h"
#include "yamicontainer.h"
#include <sstream>
#include <fstream>
#include <string>

using namespace std;
using namespace boost;
using namespace boost::asio;

namespace home_system
{

service::service(const std::string& name, bool initialize)
: name_(name),
  notify_dt_(ios_.io_service())
{
  if (initialize)
  {
    init();
  }
}

service::~service()
{
  AGENT.unregister_object(name_);
  
  notify_dt_.cancel();
  
  send_bye();
}

void service::init()
{
  AGENT.register_object(name_, *this);
  
  notify_dt_.expires_from_now(posix_time::seconds(5));
  notify_dt_.async_wait([&] (const boost::system::error_code& error) { on_notify_timeout(error); } );
  
  LOG("Started service with name: " << name_ << " and YAMI endpoint: " << ye());
  
  send_hello();
}

std::string service::name() const
{
  return name_;
}

std::string service::ye() const
{
  return YC.endpoint();
}

void service::on_msg(yami::incoming_message & im)
{
  LOGWARN("unknown message: " << im.get_message_name());
  im.reject("unknown message");
}

void service::operator()(yami::incoming_message & im)
{
//  LOG("message " << im.get_message_name() << " from " << im.get_source());
  on_msg(im);
}

// multicast send

void service::send_hello()
{
  ostringstream str;
  str << "hello\n"
    << name_ << "\n"
    << ye();
  multicast_send(str.str());
}

void service::send_notify()
{
  ostringstream str;
  str << "notify\n"
    << name_ << "\n"
    << ye();
  multicast_send(str.str());
  
  notify_dt_.cancel();
  notify_dt_.expires_from_now(posix_time::seconds(5));
  notify_dt_.async_wait( [&] (const boost::system::error_code& error) { on_notify_timeout(error); } );
}

void service::send_bye()
{
  ostringstream str;
  str << "bye\n"
    << name_;
  multicast_send(str.str());
}

void service::on_notify_timeout(const boost::system::error_code& error)
{
  if (!error)
  {
    send_notify();
  }
}

}
