#pragma once

#ifndef _MAW_BACKEND
#warning "_maw.hpp file is backend-only file. Prefer using exposed macros in maw.hpp, or, define _MAW_BACKEND (deprecated!)"
#endif

#if defined(_WIN32) || defined(_WIN64)
# define MAW_PLATFORM_WINDOWS
# define MAW_EXPORT extern "C" __declspec(dllexport)
#elif defined(__APPLE__)
# define MAW_PLATFORM_MAC
# define MAW_EXPORT extern "C" __attribute__((visibility("default")))
#else
# define MAW_PLATFORM_LINUX
# define MAW_EXPORT extern "C" __attribute__((visibility("default")))
#endif

#if defined(_WIN32)
# include <windows.h>
# define MAW_PLATFORM_EXT ".dll"
#else
# include <dlfcn.h>
# if defined(__APPLE__)
#  define MAW_PLATFORM_EXT ".dylib"
# else
#  define MAW_PLATFORM_EXT ".so"
# endif
#endif

#define MAW_DefEmptyTypeInfoCode(T, F)                      \
  inline const maw::type_info &get_type() const override {  \
    static maw::type_info info {                            \
      m_type_name<T>(),                                     \
        [] { return std::make_shared<T>(); },               \
        {},                                                 \
        F                                                   \
      };                                                    \
    return info;                                            \
  }
#define MAW_DefMethod(CLASS, NAME, ARGS, BODY)                                  \
  std::shared_ptr<maw::object> NAME ARGS BODY                                   \
  struct _maw_reg_##CLASS##_##NAME##_struct {                                   \
    _maw_reg_##CLASS##_##NAME##_struct() {                                      \
      const maw::type_info &info = CLASS().get_type();                          \
      const_cast<std::unordered_map<std::string, std::shared_ptr<maw::invocable>>&>(info.methods)[#NAME] = \
        std::make_shared<maw::invocable>([](maw::object_argv argv) -> maw::shared_object { \
          if (argv.empty()) throw maw::bad_invocation({"argc: expected self"});  \
          auto self = std::dynamic_pointer_cast<CLASS>(argv[0]);                \
          if (!self) throw maw::invalid_type({"argv[0]", "expected " #CLASS});  \
          /* call the actual C++ method (unwrap args manually here if needed) */ \
          if constexpr (std::is_same_v<decltype(&CLASS::NAME), std::shared_ptr<maw::object>(CLASS::*)(void)>) \
            return self->NAME();                                                \
          else return self->NAME(argv);                                         \
        });                                                                     \
    }                                                                           \
  } _maw_reg_instance_##CLASS##_##NAME;
#define MAW_GetSourceString(T) std::string(m_type_name(T)) + "." + std::string(__func__)
#define MAW_GetSourceStringNotFunc(T) std::string(std::string(#T)) + "." + std::string(__func__)
#define MAW_GetSrouceStringClass() MAW_GetSourceString(*this)
#define MAW_RegisterMethod(CLASS, NAME, BODY)                                   \
  namespace {                                                                   \
    struct _maw_reg_##CLASS##_##NAME {                                          \
      _maw_reg_##CLASS##_##NAME() {                                             \
        const maw::type_info &info = CLASS().get_type();                        \
        const_cast<std::unordered_map<std::string, std::shared_ptr<maw::invocable>>&>(info.methods)[#NAME] = \
          std::make_shared<maw::invocable>([](maw::object_argv argv) -> maw::shared_object BODY); \
      }                                                                         \
    } _maw_reg_instance_##CLASS##_##NAME;                                       \
  }

#define MAW_ReflectionPublicInterfaceName Maw_ReflectionEntry_0A
#define MAW_ReflectionPublicInterfaceNameStr ("Maw_ReflectionEntry_0A")

#ifdef MAW_USES_LANGUAGE_MACROS
# define cla(N) class N : public maw::object
# define let    auto
# define fun(IN, N, ARG, BODY) MAW_DefMethod(IN, N, ARG, BODY)
# define pub    public:
# define prv    private:
# define pro    protected:
# define ret    return 
# define leave  return std::make_shared<null>()
#endif
