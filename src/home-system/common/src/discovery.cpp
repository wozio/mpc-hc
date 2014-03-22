#include "discovery.h"
#include "mcs.h"
#include "logger.h"
#include "yamicontainer.h"
#include "ios_wrapper.h"
#include "service.h"
#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <sstream>
#include <string>

using namespace std;
//using namespace boost;
using namespace boost::asio;

namespace home_system
{

discovery& discovery::instance()
{
  static discovery d;
  return d;
}

discovery::discovery()
: idle_dt_(ios_.io_service()),
#ifdef _DEBUG
  listen_endpoint_(ip::udp::v4(), 10001),
#else
  listen_endpoint_(ip::udp::v4(), 10000),
#endif
  listen_socket_(ios_.io_service(), listen_endpoint_.protocol())
{
  listen_socket_.set_option(ip::tcp::socket::reuse_address(true));
  listen_socket_.bind(listen_endpoint_);
  listen_socket_.set_option(ip::multicast::join_group(
    ip::address::from_string("239.255.255.255")));

  listen_socket_.async_receive(buffer(data_),
    [&] (const boost::system::error_code& error, size_t bytes_recvd) { handle_receive(error, bytes_recvd); });

  idle_dt_.expires_from_now(boost::posix_time::seconds(10));
  idle_dt_.async_wait([this] (const boost::system::error_code& error) { on_idle_timeout(error); });
}

discovery::~discovery()
{
  listen_socket_.cancel();
  idle_dt_.cancel();
}

std::string discovery::get(const std::string& name)
{
  auto i = known_services_.find(name);
  if (i != known_services_.end())
    return i->second;

  // sending search request
  //ostringstream str;
  //str << "search\n" << name << "\n";
  //multicast_send(str.str());
    
  throw service_not_found();
}

void discovery::get_all(std::map<std::string,std::string>& services)
{
  services.clear();
  services = get_all();
}

std::map<std::string, std::string> discovery::get_all()
{
  return known_services_;
}

void discovery::subscribe(service* s)
{
  on_service_subscriptions.insert(s);
}

void discovery::unsubscribe(service* s)
{
  on_service_subscriptions.erase(s);
}

size_t discovery::subscribe(subsription callback)
{
  // find first free key, not really efficient but in this system it stops at 0 for 99% cases
  size_t key = 0;
  while (subscriptions_.find(key) != subscriptions_.end())
  {
    key++;
  }
  subscriptions_[key] = callback;
  return key;
}

void discovery::unsubscribe(size_t subscription_id)
{
  auto i = subscriptions_.find(subscription_id);
  if (i != subscriptions_.end())
  {
    subscriptions_.erase(i);
  }
}

// multicast receive, idle timer

void discovery::on_idle_timeout(const boost::system::error_code& error)
{
  if (!error)
  {
    idle_dt_.expires_at(idle_dt_.expires_at() + boost::posix_time::seconds(10));
    idle_dt_.async_wait([&] (const boost::system::error_code& error) { on_idle_timeout(error); });

    vector<string> to_remove;

    // checking if our known services sent notify within idle period (10s)
    for (map<string, bool>::iterator i = notify_received_.begin();
      i != notify_received_.end();
      ++i)
    {
      if (i->second)
        i->second = false;
      else
        to_remove.push_back(i->first);
    }
    // erasing timed out services
    for (size_t i = 0; i < to_remove.size(); ++i)
      erase_service(to_remove[i]);
  }
}

void discovery::handle_receive(const boost::system::error_code& error,
  size_t bytes_recvd)
{
  if (!error && bytes_recvd)
  {
    /*cout << endl << "=========================================================" << endl;
    cout << posix_time::microsec_clock::local_time() << endl;
    cout.write(data_, bytes_recvd);
    cout << endl;*/
    
    vector<string> fields;
    string data(&data_[0], bytes_recvd);
    
    listen_socket_.async_receive(buffer(data_, msg_max_size),
      [&] (const boost::system::error_code& error, size_t bytes_recvd) { handle_receive(error, bytes_recvd); });

    boost::split(fields, data, boost::is_any_of("\n"));

    if (fields[0] == "hello")
    {
      handle_hello(fields);
    }
    else if (fields[0] == "notify")
    {
      handle_notify(fields);
    }
    else if (fields[0] == "bye")
    {
      handle_bye(fields);
    }
    else if (fields[0] == "search")
    {
      handle_search(fields);
    }
  }
}

void discovery::handle_hello(const vector<string>& fields)
{
  if (fields.size() >= 3)
  {
    store_service(fields[1], fields[2]);
    send_notify();
  }
}

void discovery::handle_notify(const vector<string>& fields)
{
  if (fields.size() >= 3)
  {
    store_service(fields[1], fields[2]);
  }
}

void discovery::handle_bye(const vector<string>& fields)
{
  if (fields.size() >= 1)
  {
    erase_service(fields[1]);
  }
}

void discovery::handle_search(const std::vector<std::string>& fields)
{
  if (fields.size() >= 1)
  {
    send_notify();
  }
}

void discovery::send_notify()
{
  for (auto i = known_services_.begin();
    i != known_services_.end(); ++i)
  {
    ostringstream str;
    str << "notify\n"
      << i->first << "\n"
      << i->second;
    multicast_send(str.str());
  }
}

void discovery::store_service(const std::string& name, const std::string& ye)
{
  lock_guard<mutex> lock(idle_mutex);
  
  if (known_services_.find(name) == known_services_.end())
  {
    LOG("storing service: " << name << " (" << ye << ")");

    known_services_[name] = ye;
    for (auto i = on_service_subscriptions.begin();
      i != on_service_subscriptions.end();
      ++i)
    {
      if ((*i)->name() != name)
        (*i)->on_remote_service_availability(name, true);
    }
    
    for (size_t i = 0; i < subscriptions_.size(); ++i)
    {
        subscriptions_[i](name, true);
    }
  }
  notify_received_[name] = true;
}

void discovery::erase_service(const std::string& name)
{
  lock_guard<mutex> lock(idle_mutex);
  
  if (known_services_.find(name) != known_services_.end())
  {
    LOG("erasing service: " << name);
    known_services_.erase(name);
    notify_received_.erase(name);
    
    for (auto i = on_service_subscriptions.begin();
      i != on_service_subscriptions.end();
      ++i)
    {
      if ((*i)->name() != name)
        (*i)->on_remote_service_availability(name, false);
    }
    for (size_t i = 0; i < subscriptions_.size(); ++i)
    {
        subscriptions_[i](name, false);
    }
  }
}

}

