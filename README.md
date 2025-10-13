# Maw Reflection Library

## Overview

**Maw** is a simple, header-only C++ library that brings a reflection system to C++. Inspired by C#'s `System.Reflection`, Maw allows you to inspect types, create objects dynamically, and invoke methods at runtime—all in standard C++.

- **Header-only**: Just include the headers, no linking required.
- **Cross-platform**: Works on Windows, Linux, and macOS.
- **Extensible**: Add your own types and methods to the reflection system.
- **Dynamic loading**: Load types from shared libraries at runtime.

---

## Getting Started

### 1. Installation

Just copy the Maw headers into your project. No build step is required.

```cpp
#include "maw.hpp"
```

### 2. Basic Usage

To make a class reflectable, inherit from `maw::object` or use Maw's literal/typed wrappers for primitive types.

```cpp
#include "maw.hpp"

class MyClass : public maw::object {
public:
    int value = 42;
    // Add reflection methods here
};
```

You can also use Maw's built-in wrappers for primitive types:

```cpp
maw::typed<int> myInt(123);
auto typeInfo = myInt.get_type();
std::cout << "Type: " << typeInfo.fullname << std::endl;
```

### 3. Invoking Methods

You can register methods using Maw macros (see _maw.hpp for details), and invoke them dynamically:

```cpp
// Example: Registering and invoking a method
// (See MAW_DefMethod and MAW_RegisterMethod macros)
```

### 4. Dynamic Loading

Maw supports loading types from shared libraries at runtime:

```cpp
maw::assembly::dynamic_assembly asmLib("myplugin");
// Now you can access types registered in the plugin
```

---

## Key Concepts

### Types

- `maw::object`: The base class for all reflectable objects.
- `maw::literal<T>`: Wraps a value of type `T` for reflection.
- `maw::typed<T>`: Like `literal<T>`, but with stricter type checks.
- `maw::optional_typed<T>`: Like `typed<T>`, but allows null/default construction.

### Reflection

- `maw::type_info`: Holds metadata about a type (name, methods, base type).
- `maw::invocable`: Represents a callable function/method.
- `maw::function`, `maw::lambda`: Callable wrappers.

### Exceptions

- `maw::exception`: Base exception type.
- `maw::bad_invocation`, `maw::invalid_type`: Specific error types for reflection errors.

---

## Example

```cpp
#include "maw.hpp"

int main() {
  maw::typed<int> x = (10);
  std::cout << "Type: " << x.get_type().fullname << std::endl;

  auto obj = std::make_shared<maw::typed<std::string>>("Hello");
  std::cout << "Type: " << obj->get_type().fullname << std::endl;
}
```
