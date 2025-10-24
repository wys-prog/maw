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

#include "maw.object.hpp"

namespace maw {
  template <typename T>
  class literal : public object {
  protected:
    T self{};

  public:
    inline literal() = default;
    template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
    inline literal(U&& u) : self(T(std::forward<U>(u))) {}
    inline literal(const T &v) : self(v) {}
    inline literal(const std::shared_ptr<object> &obj) {
      auto v = std::dynamic_pointer_cast<literal<T>>(obj);
      if (v) {
        self = (*v).get();
      } else {
        // literal<T> can't throw when types mismatches.
      }
    }

    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<literal<T>>();
    }

    operator T() const { return self; }
    operator T&() { return self; }
    T &get() { return self; }
    T get() const { return self; }

    inline const type_info &get_type() const override {
      static type_info info{ 
        m_type_name<literal<T>>(), 
        [] (object_argv) { return std::make_shared<literal<T>>(); },
        {},
        {},
        &object().get_type()
      };
      return info;
    }
  };
}