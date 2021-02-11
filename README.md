# Simple-Log C++20 library
![Build & Test - MSVC](https://github.com/DNKpp/Simple-Log/workflows/Build%20&%20Test%20-%20MSVC/badge.svg)
![Build & Test - Clang-Cl](https://github.com/DNKpp/Simple-Log/workflows/Build%20&%20Test%20-%20Clang-Cl/badge.svg)
![Build & Test - Clang-10](https://github.com/DNKpp/Simple-Log/workflows/Build%20&%20Test%20-%20Clang-10/badge.svg)
![Build & Test - GCC-10](https://github.com/DNKpp/Simple-Log/workflows/Build%20&%20Test%20-%20GCC-10/badge.svg)

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/55561677ef904762a567b432eb4382b9)](https://www.codacy.com/gh/DNKpp/Simple-Log/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=DNKpp/Simple-Log&amp;utm_campaign=Badge_Grade)

## Author
Dominic Koepke  
Mail: [DNKpp2011@gmail.com](mailto:dnkpp2011@gmail.com)

## License

[BSL-1.0](https://github.com/DNKpp/Simple-Log/blob/master/LICENSE_1_0.txt) (free, open source)

```text
          Copyright Dominic Koepke 2021 - 2021.
 Distributed under the Boost Software License, Version 1.0.
    (See accompanying file LICENSE_1_0.txt or copy at
          https://www.boost.org/LICENSE_1_0.txt)
```

## Description
Currently under development, thus not stable.

## Installation with CMake
This library can easily be integrated into your project via CMake target_link_libraries command.

```cmake
target_link_libraries(
	<your_target_name>
	PRIVATE
	simple_log
)
```
This will add the the include path "<simple_log_install_dir>/include", thus you are able to include all headers via

```cpp
#include <Simple-Log/Simple-Log.hpp>
```

## Simple usage example
```cpp
#include <Simple-Log/Simple-Log.hpp>

#include <iostream>
#include <memory>

inline std::unique_ptr<sl::log::Core> makeLoggingCore()
{
	/* an sl::log::Core is non-copy and non-movable, thus we will store it in an unique_ptr, so we
	 * can safely move it into our global.*/
	auto core = std::make_unique<sl::log::Core>();

	/* register a BasicSink and link it to the std::cout stream. This will simply print all incoming
	messages onto the console.*/
	core->makeSink<sl::log::BasicSink>(std::cout);
	return core;
}

/* For conveniences we will simply store the core and default logger as a global. Feel free to do it otherwise.
Just make sure the Core instance doesn't get destructed before all related Logger instances.*/
inline std::unique_ptr<sl::log::Core> gLoggingCore{ makeLoggingCore() };
inline sl::log::Logger gLog{ *gLoggingCore, sl::log::SeverityLevel::info };

int main()
{
	gLog() << "Hello, World!";	// This will print this message with the "info" severity
	// creates a record with severity level "debug".
	gLog() << "Mighty debug message" << sl::log::SetSev{ sl::log::SeverityLevel::debug };
}

/*Core will make sure, that all pending Records will be processed before it gets destructed.*/
```
