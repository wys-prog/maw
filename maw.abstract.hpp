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

// Maw.Abstract;

#pragma once

#include <string>
#include <memory>
#include <unordered_map>

#include "maw.hpp"

namespace maw {
  /* Represents an abstracted type. Used as a baseclass, prefer using dynamic_class<T>
     in order to create classes at the runtime. */
  template <typename T = object>
  class abstract : public enable<abstract<T>, T> {
  protected:
    std::unordered_map<std::string, std::shared_ptr<invocable>> methods;
    std::unordered_map<std::string, std::shared_ptr<object>> members;
    
  public:
    inline const type_info &get_type() const override {
      return (*this).get_type_base(methods, members);
    }

    inline virtual std::unordered_map<std::string, std::shared_ptr<invocable>> &get_methods() {
      return methods;
    }

    inline virtual std::unordered_map<std::string, std::shared_ptr<object>> &get_members() {
      return members;
    }

    inline virtual const std::unordered_map<std::string, std::shared_ptr<invocable>> &get_methods() const {
      return methods;
    }

    inline virtual const std::unordered_map<std::string, std::shared_ptr<object>> &get_members() const {
      return members;
    }
  };

  /* Represents a class created at the runtime. */
  template <typename T = object>
  class dynamic_class : public abstract<T> {
  protected:
    std::string_view name;

    activator_t activator = [this](object_argv args) -> shared_object {
      std::string sname = std::string(name);
      if (this->get_methods().find(sname) != this->get_methods().end()) {
        (*(*this).get_methods()[sname]).invoke(args);
      }

      return std::make_shared<null>();
    };

    dynamic_class(const std::string_view &nameview, 
    const std::unordered_map<std::string, std::shared_ptr<invocable>> &mt,
    const std::unordered_map<std::string, std::shared_ptr<object>> &mb) {
      name = nameview;
      this->get_members() = mt;
      this->get_members() = mb;
    }

  public:
    dynamic_class() {}

    inline const type_info &get_type() const override {
      static type_info info(name, activator, this->get_methods(), this->get_members(), &T().get_type());

      return info;
    }
  };

  template <typename T>
  class class_template : public dynamic_class<T> {
  protected:
    std::string name_holder;
  public:
    template <typename MT, typename = std::enable_if_t<std::is_base_of_v<object, MT>>>
    inline void add_member(const std::string &name) {
      if (this->get_members().find(name) == this->get_members().end()) {
        this->get_members()[name] = std::make_shared<MT>();
      } else throw exception("redefinition of member", {name});
    }

    inline void add_method(const std::string &name, const std::shared_ptr<invocable> &f) {
      if (this->get_methods().find(name) == this->get_methods().end()) {
        this->get_methods()[name] = f;
      } else throw exception("redefinition of method", {name});
    }

    class_template(const std::string &n) 
      : dynamic_class<T>(), name_holder(n) {
        this->name = n;
      }
  };
} // namespace maw
