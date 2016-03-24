#pragma once

class addon
{
public:
  virtual ~addon() {};
  virtual int test() = 0;
};

typedef addon* (__cdecl *create_addon_t)();
