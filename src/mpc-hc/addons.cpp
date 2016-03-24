#include "stdafx.h"
#include "addons.h"
#include "../Addons/include/mpc-hc-addon/addon.h"

namespace addons
{
  void addons::init()
  {
    static addons instance;
  }

  addons::addons()
  {
    HINSTANCE hdll = LoadLibrary(TEXT("addons/addon.demo.dll"));
    if (hdll == NULL)
    {
      OutputDebugString(TEXT("AAAAAA\n"));
    }
    create_addon_t create_addon = (create_addon_t)GetProcAddress(hdll, "create_addon");
    if (create_addon == NULL)
    {
      OutputDebugString(TEXT("BBBBBB\n"));
    }

    std::unique_ptr<addon> a(create_addon());

    if (a->test() == 123)
    {
      OutputDebugString(TEXT("HAHAHAHAHAH\n"));
    }

    a.reset();

    FreeLibrary(hdll);
  }

  addons::~addons()
  {

  }
}