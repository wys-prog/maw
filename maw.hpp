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
#include <iostream>
#include <cassert>
#include <functional>
#include <filesystem>
#include <string_view>
#include <unordered_map>

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

template<typename T>
constexpr std::string_view m_type_name() {
#if defined(__clang__) || defined(__GNUC__)
    constexpr std::string_view p = __PRETTY_FUNCTION__;
    constexpr std::string_view key = "T = ";
    auto start = p.find(key);
    if (start == std::string_view::npos) return "unknown";
    start += key.size();
    auto end = p.find_first_of(";]", start);
    return p.substr(start, end - start);
#elif defined(_MSC_VER)
    constexpr std::string_view p = __FUNCSIG__;
    constexpr std::string_view key = "m_type_name<";
    auto start = p.find(key);
    if (start == std::string_view::npos) return "unknown";
    start += key.size();
    auto end = p.find_first_of(">", start);
    return p.substr(start, end - start);
#endif
}

template<typename T>
constexpr std::string_view m_type_name(const T &) { return m_type_name<T>(); } 
namespace maw {
  struct object;
  class invocable;

  using object_argv = const std::vector<std::shared_ptr<object>> &;
  using shared_object = std::shared_ptr<object>;

  struct type_info {
    std::string_view fullname;
    std::function<std::shared_ptr<object>()> activator;
    mutable std::unordered_map<std::string, std::shared_ptr<invocable>> methods;
    const type_info *basetype;

    inline type_info(std::string_view n, 
      std::function<std::shared_ptr<object>()> a, 
      const std::unordered_map<std::string, std::shared_ptr<invocable>> &m,
      const type_info *base = nullptr)
      : fullname(n), activator(std::move(a)), methods(m), basetype(base) {}

    bool operator==(const type_info &other) const { return fullname == other.fullname; }
  };

  struct object : std::enable_shared_from_this<object> {
    inline virtual std::shared_ptr<object> activator() const {
      return std::make_shared<object>();
    }

    inline virtual const type_info &get_type() const {
      static type_info info {
        m_type_name<object>(),
        [] { return std::make_shared<object>(); },
        {}
      };
      return info;
    }
  };

  struct null : public object {
    inline const type_info &get_type() const override {
      static type_info info {
        m_type_name<null>(), 
        [] { return std::make_shared<null>(); },
        {},
        &object().get_type()
      };
      return info;
    }
  };

  template <typename T>
  class literal : public object {
  protected:
    T self{};

  public:
    inline literal() = default;
    template <typename U, typename = std::enable_if_t<std::is_constructible_v<T, U&&>>>
    inline literal(U&& u) : self(T(std::forward<U>(u))) {}
    inline literal(const T &v) : self(v) {}

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
        [] { return std::make_shared<literal<T>>(); },
        {},
        &object().get_type()
      };
      return info;
    }
  };

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
        [] { return std::make_shared<invocable>(); },
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
        &exception().get_type()
      };

      return info;
    }
  };

  class invalid_type : public exception {
  public:
    inline invalid_type() : exception("invalid type") {}
    inline invalid_type(const std::vector<std::string> &argv) : exception("invalid type", argv) {}

    MAW_DefEmptyTypeInfoCode(invalid_type, &exception().get_type())
  };

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

  template <typename T>
  std::shared_ptr<T> cast_object(const std::shared_ptr<object> &ptr) {
    std::shared_ptr<T> self = std::dynamic_pointer_cast<T>(ptr);
    if (self) return self;
    throw invalid_type({__func__, "expected type " + std::string(m_type_name<T>()), MAW_GetSourceStringNotFunc(maw)});
  }

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
        [] { return std::make_shared<function>(); },
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

  template <typename T>
  std::shared_ptr<object> wrap_object(const T &obj) {
    return std::make_shared<T>(obj);
  }

  template <typename T>
  std::shared_ptr<object> wrap_object(const std::shared_ptr<T> &obj) {
    return (obj);
  }

  namespace assembly {
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
  } // namespace assembly

#pragma region common_type_definition

  typedef uint64_t    uinteger;
  typedef int64_t     integer;
  typedef bool        boolean;
  typedef long double number;
  typedef uint64_t    sizesc;

#pragma endregion common_type_definition

} // namespace maw