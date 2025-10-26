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

#include "_maw.hpp"
#include "maw.reflection.hpp"
#include "maw.object.hpp"
#include "maw.null.hpp"
#include "maw.literal.hpp"
#include "maw.exception.hpp"
#include "maw.typed.hpp"


namespace maw {
  class invocable : public object {
  public:
    using unmanaged_lambda = std::shared_ptr<object>(*)(const std::vector<std::shared_ptr<object>>&);

  protected:
    unmanaged_lambda self = nullptr;

  public:
    inline invocable() = default;
    inline invocable(unmanaged_lambda lam) : self(lam) {}

    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<invocable>();
    }

    inline virtual bool isvalid() const { return self != nullptr; }

    inline virtual std::shared_ptr<object> invoke(const std::vector<std::shared_ptr<object>> &args) const {
      if (!self) throw std::runtime_error("invocable: null target");
      return self(args);
    }

    inline virtual unmanaged_lambda target() const { return self; }
    inline virtual void target(unmanaged_lambda lam) { self = lam; }

    inline const type_info &get_type() const override {
      static type_info info {
        m_type_name<invocable>(),
        [] (object_argv) { return std::make_shared<invocable>(); },
        {
          {
            "isvalid", std::make_shared<invocable>([](auto argv) -> std::shared_ptr<object> {
              /* parse arguments */
              if (argv.size() < 1) throw bad_invocation({"argc: expected self", MAW_GetSourceString(invocable())});
              if ((*argv[0]).get_type() == invocable().get_type()) {
                auto self_obj = argv[0];
                auto self_inv = std::dynamic_pointer_cast<invocable>(self_obj);
                if (!self_inv)
                  throw bad_invocation({"argv[0]: invalid type (expected invocable)", MAW_GetSourceString(invocable())});
                else return std::make_shared<literal<bool>>((*self_inv).isvalid());
              } else throw bad_invocation({"argv: invalid type", MAW_GetSourceString(invocable())});

              return std::make_shared<null>();
            })
          },
          {
            "invoke", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              if (argv.size() < 1) throw bad_invocation({"argc: expected self", MAW_GetSourceString(invocable())});
              if ((*argv[0]).get_type() == invocable().get_type()) {
                auto self_obj = argv[0];
                auto self_inv = std::dynamic_pointer_cast<invocable>(self_obj);
                if (!self_inv)
                  throw bad_invocation({"argv[0]: invalid type (expected invocable)", MAW_GetSourceString(invocable())});
                else 
                  if (argv.size() > 1) return (*self_inv).invoke(std::vector<shared_object>(argv.begin()+1, argv.end()));
                  else return (*self_inv).invoke(std::vector<shared_object>());
              } else throw bad_invocation({"argv: invalid type", MAW_GetSourceString(invocable())});

              return std::make_shared<null>();
            })
          }, 
          {
            "target", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              if (argv.size() < 1) throw bad_invocation({"argc: expected self", MAW_GetSourceString(invocable())});
              if ((*argv[0]).get_type() == invocable().get_type()) {
                auto self_obj = argv[0];
                auto self_inv = std::dynamic_pointer_cast<invocable>(self_obj);
                if (!self_inv)
                  throw bad_invocation({"argv[0]: invalid type (expected invocable)", MAW_GetSourceString(invocable())});
                else return std::make_shared<literal<unmanaged_lambda>>((*self_inv).target());
              } else throw bad_invocation({"argv: invalid type", MAW_GetSourceString(invocable())});
              return std::make_shared<null>();
            })
          }
        },
        {},
        &exception().get_type()
      };

      return info;
    }
  };
  
  class function : public invocable {
  private:
    std::string_view fname;

  public:
    inline std::string_view name() const { return fname; }

    inline static std::shared_ptr<object> function_call(object_argv argv) {
      if (argv.size() < 1) throw bad_invocation({"argc: expected self", MAW_GetSourceString(function())});

      if ((*argv[0]).get_type() == invocable().get_type()) {
        auto self_obj = argv[0];
        auto self_inv = std::dynamic_pointer_cast<invocable>(self_obj);
        if (!self_inv) throw bad_invocation({"argv[0]: invalid type (expected invocable)", MAW_GetSourceString(function())});
        else {
          return (*self_inv).invoke(
            argv.size() > 1 ?
            std::vector<std::shared_ptr<object>>(argv.begin()+1, argv.end()) :
            std::vector<std::shared_ptr<object>>()
          );
        }
      } else throw bad_invocation({"argv: invalid type", MAW_GetSourceString(function())});
    }

    inline const type_info &get_type() const override {
      static type_info info{ 
        m_type_name<function>(), 
        [] (object_argv) { return std::make_shared<function>(); },
        {
          {
            "invoke", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              return function::function_call(argv);
            })
          },
          {
            "name", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              if (argv.size() < 1) throw bad_invocation({"argc: expected self", MAW_GetSourceString(function())});

              if ((*argv[0]).get_type() == function().get_type()) {
                auto self_obj = argv[0];
                auto self_inv = std::dynamic_pointer_cast<function>(self_obj);
                if (!self_inv) throw bad_invocation({"argv[0]: invalid type (expected invocable)", MAW_GetSourceString(function())});
                else {
                  return std::make_shared<literal<std::string_view>>((*self_inv).name());
                }
              } else throw bad_invocation({"argv: invalid type", MAW_GetSourceString(function())});
            })
          }
        }, 
        {},
        &invocable().get_type()
      };
      
      return info;
    }
  };

  class lambda : public invocable {
    using thunk_t = shared_object (*)(void*, object_argv);
    void* self = nullptr;
    thunk_t thunk = nullptr;
    void (*deleter)(void*) = nullptr;

  public:
    lambda() = default;

    template<typename F>
    lambda(F &&func) {
      using Fn = std::decay_t<F>;
      self = new Fn(std::forward<F>(func));

      thunk = [](void *ptr, object_argv argv) -> shared_object {
        return (*static_cast<Fn*>(ptr))(argv);
      };

      deleter = [](void *ptr) {
        delete static_cast<Fn*>(ptr);
      };
    }

    lambda(const lambda&) = delete;
    lambda &operator=(const lambda&) = delete;

    lambda(lambda &&other) noexcept {
      std::swap(self, other.self);
      std::swap(thunk, other.thunk);
      std::swap(deleter, other.deleter);
    }

    lambda &operator=(lambda &&other) noexcept {
      if (this != &other) {
        reset();
        std::swap(self, other.self);
        std::swap(thunk, other.thunk);
        std::swap(deleter, other.deleter);
      }
      return *this;
    }

    ~lambda() { reset(); }

    void reset() {
      if (self && deleter) deleter(self);
      self = nullptr;
      thunk = nullptr;
      deleter = nullptr;
    }

    shared_object invoke(object_argv argv) const override {
      if (!thunk) throw std::runtime_error("lambda: null target");
      return thunk(self, argv);
    }

    bool isvalid() const override { return thunk != nullptr; }

    MAW_DefEmptyTypeInfoCode(lambda, &invocable().get_type())
  };

}