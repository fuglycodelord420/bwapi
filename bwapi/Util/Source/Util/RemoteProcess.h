#pragma once
////////////////////////////////////////////////////////////////////////
//  A priviledged system handle to a remote process                   //
//                                                                    //
////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

namespace Util { class RemoteProcess; }

#include "RemoteProcessID.h"
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

namespace Util
{
  class RemoteProcess
  {
  public:
    RemoteProcess();
    ~RemoteProcess();

    bool acquire(RemoteProcessID, bool moreAccess = false);
    int exportOwnHandle();
    void importHandle(int handle);
    void release();
    HANDLE getHandle() const;
    bool isValid() const;

  private:
    HANDLE processHandle;
  };
}

#endif
