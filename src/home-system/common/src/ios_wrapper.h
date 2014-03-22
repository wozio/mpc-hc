#ifndef IOS_WRAPPER_H
#define	IOS_WRAPPER_H

#include <boost/asio.hpp>
#include <thread>
#include <mutex>
#include <memory>

namespace home_system
{

class ios_wrapper
{
public:
  ios_wrapper();
  ~ios_wrapper();
  ios_wrapper(const ios_wrapper&) = delete;
  ios_wrapper& operator=(const ios_wrapper&) = delete;
  
  boost::asio::io_service& io_service();
private:
  boost::asio::io_service io_service_;
  std::thread io_thread_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  
  void thread_exec();
};

}

#endif	/* IOS_WRAPPER_H */

