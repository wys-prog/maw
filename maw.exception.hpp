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

namespace maw {
  class exception : public object, public std::exception { /* can be caught by catch(const std::exception&) expressions. */
  protected:
    std::string _what;

  public:
    inline exception() = default;
    inline exception(const std::string &arg) : _what(arg) {}
    inline exception(const std::string &arg, const std::vector<std::string> &argv)
      : _what("'" + arg + "'") { for (const auto &a:argv) _what += " | " + a;}
    inline exception(const exception &e) : _what(e.what()) {}

    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<exception>();
    }

    inline const char *what() const noexcept override {
      return _what.c_str();
    }

    inline static literal<std::string> msg(const std::shared_ptr<object> &self) {
      auto v_self = std::dynamic_pointer_cast<exception>(self);
      if (v_self) { return literal<std::string>((*v_self).what()); }
      return literal<std::string>("bad object: given argument hasn't " + std::string(m_type_name<exception>()) + " as base class");
    }

    inline const type_info &get_type() const override {
      static type_info info{ 
        m_type_name<exception>(), 
        [] { return std::make_shared<exception>(); },
        {
          {"msg", std::make_shared<invocable>([](object_argv argv) -> shared_object {
            if (argv.size() >= 1) return std::make_shared<object>(exception::msg(argv[0]));
            return std::make_shared<null>();
          })}
        },
        &object().get_type()
      };
      return info;
    }
  };

  class bad_invocation : public exception {
  public:
    inline bad_invocation() : exception("bad invocation") {}
    inline bad_invocation(const std::vector<std::string> &s) : exception("bad invocation", s) {}

    MAW_DefEmptyTypeInfoCode(bad_invocation, &exception().get_type())
  };

  class invalid_type : public exception {
  public:
    inline invalid_type() : exception("invalid type") {}
    inline invalid_type(const std::vector<std::string> &argv) : exception("invalid type", argv) {}

    MAW_DefEmptyTypeInfoCode(invalid_type, &exception().get_type())
  };
}