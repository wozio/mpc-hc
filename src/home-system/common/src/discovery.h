#ifndef DISCOVERY_H
#define	DISCOVERY_H

#include "ios_wrapper.h"
#include <functional>
#include <map>
#include <set>

#define DISCOVERY home_system::discovery::instance()

namespace home_system
{

class service;

typedef std::function<void (const std::string&, bool) > subsription;
typedef std::map<size_t, subsription> subsriptions;

class discovery
{
public:
  static discovery& instance();
  ~discovery();

  void thread_exec();

  std::string get(const std::string& name);
  void get_all(std::map<std::string, std::string>& services);
  std::map<std::string, std::string> get_all();

  void subscribe(service* s);
  void unsubscribe(service* s);

  size_t subscribe(subsription callback);
  void unsubscribe(size_t subscription_id);

  void on_connection_closed(const char* endpoint);

private:
  discovery();

  // known external services (key is name, value is yamie endpoint)
  std::map<std::string, std::string> known_services_;

  // external services supervision
  // notify received from service within last idle period (key is id)
  std::map<std::string, bool> notify_received_;
  home_system::ios_wrapper ios_;
  boost::asio::deadline_timer idle_dt_;
  std::mutex idle_mutex;

  void on_idle_timeout(const boost::system::error_code& error);


  void store_service(const std::string& name, const std::string& ye);
  void erase_service(const std::string& name);

  // multicast receive
  boost::asio::ip::udp::endpoint listen_endpoint_;
  boost::asio::ip::udp::socket listen_socket_;

  enum
  {
    msg_max_size = 65507
  };
  char data_[msg_max_size];

  void handle_receive(const boost::system::error_code& error, size_t bytes_recvd);

  void handle_hello(const std::vector<std::string>& fields);
  void handle_notify(const std::vector<std::string>& fields);
  void handle_bye(const std::vector<std::string>& fields);
  void handle_search(const std::vector<std::string>& fields);

  void send_notify();

  // service avaliability subscriptions, key is name of the service
  std::set<service*> on_service_subscriptions;
  subsriptions subscriptions_;
};

class service_not_found
: public std::exception
{
};

}

#endif	/* DISCOVERY_H */

