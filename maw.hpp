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

#define _MAW_BACKEND

#include "_maw.hpp"
#include "maw.reflection.hpp"
#include "maw.object.hpp"
#include "maw.null.hpp"
#include "maw.literal.hpp"
#include "maw.exception.hpp"
#include "maw.typed.hpp"
#include "maw.functionnal.hpp"

namespace maw {
  #pragma region Maw C++ Types

  namespace maw_types {
    using mc_integer   = int64_t;
    using mc_uinteger  = uint64_t;
    using mc_boolean   = int8_t;
    using mc_number    = double; // May get changes in further versions.
    using mc_byte      = uint8_t;

    using integer   = typed<mc_integer>;
    using uinteger  = typed<mc_uinteger>;
    using boolean   = typed<mc_boolean>;
    using number    = typed<mc_number>;
    using byte      = typed<mc_byte>;
  }

  #ifdef MAW_USES_MC
  using namespace maw_types;
  #endif // MAW_USES_MC_TYPES_GLOB

  #pragma endregion

  template <typename T>
  std::shared_ptr<T> cast_object(const std::shared_ptr<object> &ptr) {
    std::shared_ptr<T> self = std::dynamic_pointer_cast<T>(ptr);
    if (self) return self;
    throw invalid_type({__func__, "expected type " + std::string(m_type_name<T>()), MAW_GetSourceStringNotFunc(maw)});
  }


  template <typename T>
  std::shared_ptr<object> wrap_object(const T &obj) {
    return std::make_shared<T>(obj);
  }

  template <typename T>
  std::shared_ptr<object> wrap_object(const std::shared_ptr<T> &obj) {
    return (obj);
  }

  std::shared_ptr<literal<std::vector<std::string>>> unwrap_bases(const std::shared_ptr<object> &from) {
    std::stack<std::string> s;
    const auto &smth = (*from).get_type(); // keep reference (no copy)
    const maw::type_info* info = &smth;

    while (info) {
      s.push(std::string(info->fullname));
      info = info->basetype; // move upward
    }

    std::vector<std::string> v;
    while (!s.empty()) {
      v.push_back(s.top());
      s.pop();
    }

    return std::make_shared<literal<std::vector<std::string>>>(v);
  }
} // namespace maw

#undef _MAW_BACKEND