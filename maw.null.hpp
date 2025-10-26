//  Maw Reflection Core - by MONOE. (https://github.com/wys-prog)
//  Free and open-source software.
//
//  Usage Terms:
//  - You are free to use, modify, and redistribute this code.
//  - Please keep attribution in the source (at least mention "MONOE.").
//  - This code is provided "as is" without any warranty.
//    By using it, you accept that any issues, bugs, or failures
//    are entirely your own responsibility.
//
//  Enjoy coding <3
//  Thank you !

#pragma once

#include <any>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <functional>
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "maw.reflection.hpp"
#include "maw.object.hpp"

namespace maw {
  struct null : public object {
    inline const type_info &get_type() const override {
      static type_info info {
        m_type_name<null>(), 
        [] (object_argv) { return std::make_shared<null>(); },
        {},
        {},
        &object().get_type()
      };
      return info;
    }
  };
}