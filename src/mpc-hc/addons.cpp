#include "stdafx.h"
#include "addons.h"

namespace addons
{
  addons& addons::instance()
  {
    static addons i;
    return i;
  }

  addons::addons()
  {
    hdll_ = LoadLibrary(TEXT("addons/addon.demo.dll"));
    if (hdll_ == NULL)
    {
      return;
    }
    create_addon_t create_addon = (create_addon_t)GetProcAddress(hdll_, "create_addon");
    if (create_addon == NULL)
    {
      FreeLibrary(hdll_);
      return;
    }
    addon_.reset(create_addon());
  }

  addons::~addons()
  {
    addon_.reset();
    FreeLibrary(hdll_);
  }

  void add_menu_item()
  {

  }
}