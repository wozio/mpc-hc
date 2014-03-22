#include "ios_wrapper.h"

using namespace std;
using namespace boost::asio;

namespace home_system
{

ios_wrapper::ios_wrapper()
{
  io_thread_ = thread([this] () {thread_exec();});
}

ios_wrapper::~ios_wrapper()
{
  work_.reset();
  io_service_.stop();
  if (io_thread_.joinable())
  {
    io_thread_.join();
  }
}

boost::asio::io_service& ios_wrapper::io_service()
{
  return io_service_;
}

void ios_wrapper::thread_exec()
{
  try
  {
    work_.reset(new io_service::work(io_service_));
    io_service_.run();
  }
  catch (const std::exception& e)
  {
  }
}

}
