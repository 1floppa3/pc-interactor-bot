#pragma once

namespace Core {

  class PrivilegeHelper {
    static bool check_admin();
    static bool relaunch_as_admin();
  public:
    static void ensure_admin();
  };

}