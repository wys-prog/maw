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