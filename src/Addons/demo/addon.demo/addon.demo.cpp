#include "stdafx.h"
#include "mpc-hc-addon/addon.h"

class demo : public addon
{
public:

  demo()
  {
  }

  ~demo()
  {
  }

  int test()
  {
    return 123;
  }
};

#ifdef __cplusplus    // If used by C++ code, 
extern "C" {          // we need to export the C interface
#endif

__declspec(dllexport) addon* __cdecl create_addon()
{
  return new demo();
}

#ifdef __cplusplus
}
#endif