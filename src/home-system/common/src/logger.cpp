#include "logger.h"

#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/ConsoleChannel.h>
#include <Poco/FileChannel.h>
#include <Poco/SplitterChannel.h>
#include <Poco/AutoPtr.h>

namespace home_system
{
namespace logger
{

void configure(const char* file_name, const std::string& log_level, bool console_log)
{
  Poco::Logger &l = Poco::Logger::root();

  Poco::AutoPtr<Poco::FileChannel> sfc(new Poco::FileChannel(file_name));
  sfc->setProperty("rotation","1 M");
  sfc->setProperty("purgeCount","10");

  Poco::AutoPtr<Poco::PatternFormatter> pf(new Poco::PatternFormatter);
  pf->setProperty("pattern", "[%q] %Y-%m-%d %H:%M:%S:%i (%I)%s: %t");

  Poco::AutoPtr<Poco::FormattingChannel> fc(new Poco::FormattingChannel(pf));

  if (console_log)
  {
    Poco::AutoPtr<Poco::SplitterChannel> sc(new Poco::SplitterChannel);
    Poco::AutoPtr<Poco::ConsoleChannel> cc(new Poco::ConsoleChannel);

    sc->addChannel(cc);
    sc->addChannel(sfc);

    fc->setChannel(sc);
  }
  else
  {
    fc->setChannel(sfc);
  }

  l.setChannel(fc);
  l.setLevel(log_level);
}

}  
}
