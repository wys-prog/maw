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

namespace maw {
  struct object;
  class invocable;

  using object_argv = const std::vector<std::shared_ptr<object>> &;
  using shared_object = std::shared_ptr<object>;
  using activator_t = std::function<std::shared_ptr<object>(const std::vector<std::shared_ptr<object>>&)>;

  struct type_info {
    std::string_view                                                    fullname;
    activator_t                                                         activator;
    mutable std::unordered_map<std::string, std::shared_ptr<invocable>> methods;
    mutable std::unordered_map<std::string, std::shared_ptr<object>>    members;
    const type_info                                                    *basetype;

    inline type_info(std::string_view n, activator_t a, 
      const std::unordered_map<std::string, std::shared_ptr<invocable>> &methds,
      const std::unordered_map<std::string, std::shared_ptr<object>> &mmbrs,
      const type_info *base = nullptr)
      : fullname(n), activator(std::move(a)), methods(methds), members(std::move(mmbrs)), basetype(base) {}

    bool operator==(const type_info &other) const { return fullname == other.fullname; }
  };

  struct object : std::enable_shared_from_this<object> {
    inline virtual std::shared_ptr<object> activator() const {
      return std::make_shared<object>();
    }

    inline virtual const type_info &get_type() const {
      static type_info info {
        m_type_name<object>(),
        [] (object_argv) { return std::make_shared<object>(); },
        {},
        {},
        nullptr
      };
      return info;
    }

    inline virtual std::string to_string() const { return std::string(get_type().fullname); }
    inline virtual std::string_view to_string_view() const { return std::string_view(get_type().fullname); }
  };
}