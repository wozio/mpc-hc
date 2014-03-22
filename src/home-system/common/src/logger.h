#ifndef LOGGER_H
#define	LOGGER_H

#include <Poco/LogStream.h>
#include <Poco/Logger.h>

namespace home_system
{
namespace logger
{

void configure(const char* file_name, const std::string& log_level, bool console_log);

}
}

#define LOGDEBUG(msg) {Poco::LogStream ls(Poco::Logger::get(__FILE__)); ls.debug()       << __LINE__ << ": " << msg << std::endl;}
#define LOG(msg) LOGDEBUG(msg)
#define LOGINFO(msg)  {Poco::LogStream ls(Poco::Logger::get(__FILE__)); ls.information() << __LINE__ << ": " << msg << std::endl;}
#define LOGWARN(msg)  {Poco::LogStream ls(Poco::Logger::get(__FILE__)); ls.warning()     << __LINE__ << ": " << msg << std::endl;}
#define LOGERROR(msg) {Poco::LogStream ls(Poco::Logger::get(__FILE__)); ls.error()       << __LINE__ << ": " << msg << std::endl;}

#endif	/* LOGGER_H */

