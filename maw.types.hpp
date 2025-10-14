//  Maw Reflection Core - by Wys (https://github.com/wys-prog)
//  Free and open-source software.
//
//  Usage Terms:
//  - You are free to use, modify, and redistribute this code.
//  - Please keep attribution in the source (at least mention "Wys").
//  - This code is provided "as is" without any warranty.
//    By using it, you accept that any issues, bugs, or failures
//    are entirely your own responsibility.
//
//  Enjoy coding <3
//  Thank you !

#pragma once

#include <any>
#include <stack>
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

#include "maw.typed.hpp"

namespace maw::maw_types {
  using mc_integer   = int64_t;
  using mc_uinteger  = uint64_t;
  using mc_boolean   = int8_t;
  using mc_number    = double; // May get changes in further versions.
  using mc_byte      = uint8_t;
  using mc_len       = uint64_t;

  using integer   = typed<mc_integer>;
  using uinteger  = typed<mc_uinteger>;
  using boolean   = typed<mc_boolean>;
  using number    = typed<mc_number>;
  using byte      = typed<mc_byte>;
  using len       = typed<mc_len>;

    /* Represents an interface -- base type! */
  class interface : public object {
  public:
    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<interface>();
    }

    inline const type_info &get_type() const override {
      static type_info info {
        m_type_name<interface>(),
        [] { return std::make_shared<interface>(); },
        {},
        &object().get_type()
      };
      return info;
    }

    inline virtual bool valid() const { return false; }
    inline virtual void dispose() const { }
  };

  class informational_interface : public interface {
  public:
    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<informational_interface>();
    }

    inline const type_info &get_type() const override {
      static type_info info {
        m_type_name<informational_interface>(),
        [] { return std::make_shared<informational_interface>(); },
        {},
        &object().get_type()
      };
      return info;
    }
  };
}

#ifdef MAW_USES_MC
using namespace maw_types;
#endif // MAW_USES_MC_TYPES_GLOB