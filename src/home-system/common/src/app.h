#ifndef APP_H
#define	APP_H

#include <functional>
#include <vector>
#include <string>

namespace home_system
{

typedef std::function<void (const std::vector<std::string>&)> cmd_handler_type;

class app
{
public:
  app(bool daemonize, cmd_handler_type cmd_handler = nullptr);
  virtual ~app();
  
  int run();
  
private:
  bool daemonize_;
  cmd_handler_type cmd_handler_;
};

}

#endif	/* APP_H */

