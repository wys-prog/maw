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

// Maw.System.Assembly;
// Maw.System.Assembly.V1 is guanranteed to be the same regarding your version of Maw.System.

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
#include "maw.typed.hpp"
#include "maw.types.hpp"

namespace maw {
namespace assembly::V1 {
    class assembly_exception : public exception {
    public: 
      inline assembly_exception() : exception("assembly exception") {}
      inline assembly_exception(const std::vector<std::string> &args) : exception("assembly exception", args) {}

      inline std::shared_ptr<object> activator() const override {
        return std::make_shared<assembly_exception>();
      }

      MAW_DefEmptyTypeInfoCode(assembly_exception, &exception().get_type())
    };

    class assembly_object : public object {
    public:
      using type_map = std::unordered_map<std::string, const type_info*>;

    protected:
      type_map types_;
      std::filesystem::path path_;

    public:
      inline std::filesystem::path path() const { return path_; } 

      inline void register_type(const type_info& info) {
        types_.emplace(std::string(info.fullname), &info);
      }

      inline const type_map &types() const noexcept {
        return types_;
      }

      inline std::shared_ptr<object> activator() const override {
        return std::make_shared<assembly_object>();
      }

      assembly_object() {}
      assembly_object(const type_map &map) 
        : types_(map), path_("") {}
      assembly_object(const type_map &map, const std::filesystem::path &p) 
        : types_(map), path_(p) {}
      
      MAW_DefEmptyTypeInfoCode(assembly_object, &object().get_type())
    };
    
    class dynamic_assembly : public assembly_object {
    private:
      void* handle_ = nullptr;
      
    public:
      using register_fn = assembly_object*(*)();
      
      dynamic_assembly() = default;
      
      explicit dynamic_assembly(const std::filesystem::path &name, bool auto_platform = true) {
        load(name, auto_platform);
      }
      
      inline void load(const std::filesystem::path &name, bool auto_platform = true) {
        if (handle_) unload();
        
        std::filesystem::path lib_path = name;
        if (auto_platform && lib_path.extension().empty())
        lib_path += MAW_PLATFORM_EXT;
        
        path_ = lib_path;
        
        #if defined(_WIN32)
        handle_ = static_cast<void*>(LoadLibraryA(lib_path.string().c_str()));
        #else
        handle_ = dlopen(lib_path.string().c_str(), RTLD_NOW | RTLD_GLOBAL);
        #endif
        
        if (!handle_) {
          std::string err;
          #if defined(_WIN32)
          err = "LoadLibrary failed: " + lib_path.string();
          #else
          err = std::string(dlerror() ? dlerror() : "dlopen failed") + ": " + lib_path.string();
          #endif
          throw assembly_exception({ err });
        }
        
        register_fn reg = nullptr;
        #if defined(_WIN32)
        reg = reinterpret_cast<register_fn>(GetProcAddress(static_cast<HMODULE>(handle_), MAW_ReflectionPublicInterfaceName));
        #else
        reg = reinterpret_cast<register_fn>(dlsym(handle_, MAW_ReflectionPublicInterfaceNameStr));
        #endif
        
        if (reg) {
          try {
            auto lib_ = reg();
            for (const auto &elem:(*lib_).types()) {
              register_type(*(elem.second));
            }
          } catch (const std::exception &e) {
            throw assembly_exception({"exception when calling reflection function", e.what()});
          }
        } else {
          throw assembly_exception({"no such reflection entry in library " + lib_path.lexically_normal().string(), 
            "expected entry name: ", MAW_ReflectionPublicInterfaceNameStr});
        }
      }
      
      inline void unload() {
        if (!handle_) return;
        #if defined(_WIN32)
        FreeLibrary(static_cast<HMODULE>(handle_));
        #else
        dlclose(handle_);
        #endif
        handle_ = nullptr;
        types_.clear();
      }
      
      inline bool is_loaded() const noexcept { return handle_ != nullptr; }
      
      inline ~dynamic_assembly() { unload(); }
      
      inline std::shared_ptr<object> activator() const override {
        return std::make_shared<dynamic_assembly>();
      }
      
      inline const type_info &get_type() const override {
        static type_info info{
          m_type_name<dynamic_assembly>(),
          [] { return std::make_shared<dynamic_assembly>(); },
          {
            {
              "is_loaded", std::make_shared<invocable>([](object_argv argv) -> shared_object {
                if (argv.empty()) throw bad_invocation({"argc: expected self", MAW_GetSourceString(dynamic_assembly())});
                auto self = std::dynamic_pointer_cast<dynamic_assembly>(argv[0]);
                if (!self) throw invalid_type({"argv[0]", "expected dynamic_assembly", MAW_GetSourceString(dynamic_assembly())});
                return std::make_shared<literal<bool>>(self->is_loaded());
              })
            },
            {
              "path", std::make_shared<invocable>([](object_argv argv) -> shared_object {
                if (argv.empty()) throw bad_invocation({"argc: expected self", MAW_GetSourceString(dynamic_assembly())});
                auto self = std::dynamic_pointer_cast<dynamic_assembly>(argv[0]);
                if (!self) throw invalid_type({"argv[0]", "expected dynamic_assembly", MAW_GetSourceString(dynamic_assembly())});
                return std::make_shared<literal<std::string>>(self->path().string());
              })
            },
            {
              "load", std::make_shared<invocable>([](object_argv argv) -> shared_object {
                if (argv.size() < 2) throw bad_invocation({"argc: expected self + path", MAW_GetSourceString(dynamic_assembly())});
                auto self = std::dynamic_pointer_cast<dynamic_assembly>(argv[0]);
                auto path_obj = std::dynamic_pointer_cast<literal<std::string>>(argv[1]);
                if (!self || !path_obj) throw invalid_type({"argv", "expected dynamic_assembly + string", MAW_GetSourceString(dynamic_assembly())});
                self->load(path_obj->get());
                return std::make_shared<null>();
              })
            }
          }, &assembly_object().get_type()
        };
        return info;
      }
    };
  } // namespace assembly V1

#pragma region Assembly Types

  namespace assembly::V1::assembly_types {

    template <typename T>
    class quick_view : public object {
    private:
      const T *start;
      maw_types::mc_len size_;

    public:
      inline const T *begin() const { return start; }
      inline maw_types::mc_len size() const { return size_; }

      inline const type_info &get_type() const override {
        static type_info info {
          m_type_name<quick_view>(),
          [] { return std::make_shared<quick_view>(); },
          {},
          &object().get_type()
        };
        return info;
      }
      
      inline std::shared_ptr<object> activator() const override {
        return std::make_shared<quick_view>();
      }
    };

    template <>
    class quick_view<std::string> : public object {
    private:
      const char *start;
      maw_types::mc_len size_;

    public:
      inline const char *begin() const { return start; }
      inline maw_types::mc_len size() const { return size_; }

      inline const type_info &get_type() const override {
        static type_info info {
          m_type_name<quick_view>(),
          [] { return std::make_shared<quick_view>(); },
          {},
          &object().get_type()
        };
        return info;
      }
      
      inline std::shared_ptr<object> activator() const override {
        return std::make_shared<quick_view>();
      }
    };
  }

#pragma endregion


  #ifdef MAW_USES_SPEC_ASM_V
  // Nothing for now!
  #else
  namespace assembly {
    using namespace V1; // Import the V1
  }
  #endif
}