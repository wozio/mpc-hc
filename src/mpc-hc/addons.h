#pragma once

#include "../Addons/include/mpc-hc-addon/addon.h"

namespace addons
{
  class addons
  {
  public:
    static addons& instance();
    ~addons();
  private:
    addons();

    HINSTANCE hdll_;
    std::unique_ptr<addon> addon_;
  };
}

#define ADDONS addons::addons::instance()