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
#define MAW_SYSTEM_CXX

#include "_maw.hpp"
#include "maw.reflection.hpp"
#include "maw.object.hpp"
#include "maw.null.hpp"
#include "maw.literal.hpp"
#include "maw.exception.hpp"
#include "maw.typed.hpp"
#include "maw.functionnal.hpp"
#include "maw.assembly.hpp"

#define M_export(func, argc, mt, ...) \
  { \
  #func, std::make_shared<maw::invocable>([](maw::object_argv args) -> maw::shared_object { \
    if ((args.size() + 1) < argc) throw maw::bad_invocation({"Too few arguments. Expected " + std::to_string(argc) + " got " + std::to_string(args.size()), "from " + std::string(mt().get_type().fullname) + "." + std::string(#func)}); \
    if ((*args[0]).get_type() == mt().get_type()) { \
      auto self = std::dynamic_pointer_cast<mt>(args[0]); \
      return (*self).func(__VA_ARGS__);  \
    } throw maw::bad_invocation({"Invalid self"}); \
  }) }

#define M_decl_class(cl, bs) private: using M_INNER = bs; using M_THIS = cl;


namespace maw {
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

  template <typename C, typename B = object>
  class enable : public object {
  protected:
    using self_t  = C;
    using inner_t = B;

    inline static const type_info &get_type_base(
      const std::unordered_map<std::string, std::shared_ptr<invocable>> &methds = {},
      const std::unordered_map<std::string, std::shared_ptr<object>> &mmbrs = {}
    ) {
      static type_info info{
        m_type_name<self_t>(), 
        [] (object_argv) { return std::make_shared<self_t>(); },
        methds,
        mmbrs,
        &inner_t().get_type()
      };
      return info;
    }

    inline static void register_method(const std::string& name,
                                       std::shared_ptr<invocable> fn) {
      get_type_base().methods[name] = std::move(fn);
    }

    template <typename F>
    inline static void register_method(const std::string& name, F&& fn) {
      get_type_base().methods[name] = std::make_shared<invocable>(std::forward<F>(fn));
    }
  };
} // namespace maw

#undef _MAW_BACKEND