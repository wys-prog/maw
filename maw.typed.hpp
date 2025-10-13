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

#include "_maw.hpp"
#include "maw.reflection.hpp"
#include "maw.object.hpp"
#include "maw.null.hpp"
#include "maw.literal.hpp"
#include "maw.exception.hpp"
#include "maw.functionnal.hpp"

namespace maw {
  template <typename T>
  class typed : public literal<T> {
  public:
    inline typed() {}
    inline typed(const T &t) : literal<T>(t) {}
    inline typed(const literal<T> &lit) : literal<T>(lit) {}
    template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
    inline typed(U&& u) : literal<T>(T(std::forward<U>(u))) {}
    inline typed(const std::shared_ptr<object> &obj) {
      auto smth = std::dynamic_pointer_cast<literal<T>>(obj);
      if (!smth) throw invalid_type({"expected type " + std::string(m_type_name<literal<T>>()), MAW_GetSourceString(invocable())});
      this->self = (*smth);
    }

    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<typed<T>>();
    }

    operator T() const { return this->self; }
    operator T&() { return this->self; }

    inline const type_info &get_type() const override {
      static type_info info{ 
        m_type_name<typed<T>>(), 
        [] { return std::make_shared<typed<T>>(); },
        {},
        &literal<T>().get_type()
      };
      return info;
    }
  };

  template <typename T>
  class optional_typed : public literal<T> {
  public:
    inline optional_typed() {}
    inline optional_typed(const literal<T> &lit) : literal<T>(lit) {}
    inline optional_typed(const typed<T> &tyed) : literal<T>(tyed) {}
    template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
    inline optional_typed(U&& u) : literal<T>(T(std::forward<U>(u))) {}
    inline optional_typed(const std::shared_ptr<object> &obj) {
      auto smth = std::dynamic_pointer_cast<literal<T>>(obj);
      if (smth) this->self = (*smth);
    }

    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<optional_typed<T>>();
    }

    inline const type_info &get_type() const override {
      static type_info info{ 
        m_type_name<optional_typed<T>>(),
        [] { return std::make_shared<optional_typed<T>>(); },
        {}, 
        &literal<T>().get_type()
      };
      return info;
    }
  };
}