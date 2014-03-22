#include "app.h"

#include "logger.h"
#include <boost/algorithm/string.hpp>
#ifdef __linux__
#include <signal.h>
#include <sys/stat.h>
#endif
#include <cstdlib>
#include <cstdio>

using namespace std;

namespace home_system
{

app::app(bool daemonize, cmd_handler_type cmd_handler)
: cmd_handler_(cmd_handler),
  daemonize_(daemonize)
{
#ifdef __linux__
  if (daemonize_)
  {
    cout << "Running as daemon" << endl;
    
    pid_t pid = fork();
    if (pid < 0)
    {
      LOGERROR("Cannot fork");
      exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
      exit(EXIT_SUCCESS);
    }

    umask(0);

    pid_t sid = setsid();
    if (sid < 0)
    {
      LOGERROR("Cannot setsid");
      exit(EXIT_FAILURE);
    }
    
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
  }
#endif
}

app::~app()
{
  
}

int app::run()
{
#ifdef __linux__
  if (daemonize_)
  {
    sigset_t sset;
    sigemptyset(&sset);
    sigaddset(&sset, SIGQUIT);
    sigaddset(&sset, SIGTERM);
    sigprocmask(SIG_BLOCK, &sset, NULL);
    int sig;
    sigwait(&sset, &sig);
  }
  else
#endif
  {
    cout << "Enter q to quit..." << endl;
    std::string input_line;
    while (std::getline(std::cin, input_line))
    {
      vector<string> fields;
      boost::split(fields, input_line, boost::is_any_of(" "));
      if (fields.size() >= 1)
      {
        if (fields[0] == "q" || fields[0] == "quit")
        {
          break;
        }
        else
        {
          if (cmd_handler_)
          {
            cmd_handler_(fields);
          }
        }
      }
    }
  }
  return 0;
}
  
}
