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
#include <chrono>
#include <vector>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <functional>
#include <filesystem>
#include <string_view>
#include <unordered_map>

#include "maw.types.hpp"
#include "maw.typed.hpp"
#include "maw.assembly.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace maw::contexts {

  class current_application : public maw::maw_types::informational_interface {
  private:
    using quick_string = maw::assembly::assembly_types::quick_view<std::string>;

    std::filesystem::path exe_path_;
    std::filesystem::path exe_dir_;
    std::vector<std::string> args_;
    std::unordered_map<std::string, std::string> env_;
    std::chrono::steady_clock::time_point start_time_;
    std::string process_name_;
    uint32_t pid_;

    inline void collect_env() {
      #if defined(_WIN32)
      char* env_strings = GetEnvironmentStringsA();
      if (!env_strings) return;
      const char* var = env_strings;
      while (*var) {
        std::string entry(var);
        auto pos = entry.find('=');
        if (pos != std::string::npos) {
          env_.emplace(entry.substr(0, pos), entry.substr(pos + 1));
        }
        var += entry.size() + 1;
      }
      FreeEnvironmentStringsA(env_strings);
      #else
      extern char **environ;
      for (char **env = environ; *env; ++env) {
        std::string entry(*env);
        auto pos = entry.find('=');
        if (pos != std::string::npos) {
          env_.emplace(entry.substr(0, pos), entry.substr(pos + 1));
        }
      }
      #endif
    }

  public:
    current_application() 
      : start_time_(std::chrono::steady_clock::now()) {
      #if defined(_WIN32)
      char path[MAX_PATH];
      GetModuleFileNameA(nullptr, path, MAX_PATH);
      exe_path_ = std::filesystem::absolute(path);
      #else
      char path[4096];
      ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
      if (count != -1) {
        path[count] = '\0';
        exe_path_ = std::filesystem::absolute(path);
      }
      #endif

      exe_dir_ = exe_path_.parent_path();
      process_name_ = exe_path_.filename().string();

      #if defined(_WIN32)
      pid_ = static_cast<uint32_t>(GetCurrentProcessId());
      #else
      pid_ = static_cast<uint32_t>(getpid());
      #endif

      collect_env();
    }

    inline std::filesystem::path exe_path() const { return exe_path_; }
    inline std::filesystem::path exe_dir() const { return exe_dir_; }
    inline const std::string &process_name() const { return process_name_; }
    inline uint32_t process_id() const { return pid_; }
    inline const std::unordered_map<std::string, std::string> &environment() const { return env_; }

    inline std::string environment_variable(const std::string &key) const {
      if (auto it = env_.find(key); it != env_.end())
        return it->second;
      return {};
    }

    inline std::chrono::milliseconds uptime() const {
      return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - start_time_);
    }

    inline std::string os_name() const {
      #if defined(_WIN32)
      return "Windows";
      #elif defined(__APPLE__)
      return "macOS";
      #elif defined(__linux__)
      return "Linux";
      #elif defined(__unix__)
      return "Unix";
      #else
      return "Unknown";
      #endif
    }

    inline std::shared_ptr<object> activator() const override {
      return std::make_shared<current_application>();
    }

    inline const type_info &get_type() const override {
      static type_info info{
        m_type_name<current_application>(),
        [] (object_argv) { return std::make_shared<current_application>(); },
        {
          { "exe_path", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              return std::make_shared<literal<std::string>>(self->exe_path().string());
          }) },
          { "exe_dir", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              return std::make_shared<literal<std::string>>(self->exe_dir().string());
          }) },
          { "process_name", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              return std::make_shared<literal<std::string>>(self->process_name());
          }) },
          { "process_id", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              return std::make_shared<literal<uint32_t>>(self->process_id());
          }) },
          { "os_name", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              return std::make_shared<literal<std::string>>(self->os_name());
          }) },
          { "uptime_ms", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              return std::make_shared<literal<uint64_t>>(self->uptime().count());
          }) },
          { "env", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              std::string joined;
              for (auto &[k, v] : self->environment())
                joined += k + "=" + v + "\n";
              return std::make_shared<literal<std::string>>(joined);
          }) },
          { "get_env", std::make_shared<invocable>([](object_argv argv) -> shared_object {
              if (argv.size() < 2) throw bad_invocation({"argc: expected self + varname"});
              auto self = std::dynamic_pointer_cast<current_application>(argv[0]);
              auto key = std::dynamic_pointer_cast<literal<std::string>>(argv[1]);
              return std::make_shared<literal<std::string>>(self->environment_variable(key->get()));
          }) }
        },
        {},
        &maw::maw_types::informational_interface().get_type()
      };
      return info;
    }
  };

  static thread_local const constexpr current_application capp;
}

maw::literal<int> mawmain(const maw::typed_ptr<std::vector<std::string>>&)
#ifdef MAW_LEADS_MAIN
; /* forward declaration */

int main(int argc, char **argv) {
  maw::literal<std::vector<std::string>> vec;
  for (int i = 0; i < argc; i++) vec.get().push_back(argv[i]);
  return mawmain(std::make_shared<maw::typed<std::vector<std::string>>>(vec));
}
#else
{ /* declare so compilers won't yell lul */
  /* does nothing !*/
  return 0;
}
#endif
