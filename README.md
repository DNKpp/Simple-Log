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
This is a highly customizable multithreaded logging library, which makes heavy use of loosly coupled concepts rather than macros. Other than many other libraries, there are no singleton classes or forced global objects. It's up to the users if they
want globals or not.

If your goal is simply logging everything to console or a file, than you may want to begin with the ``<ReadyToGo>``-header, which will set-up everything you'll need to be able start logging (also look at the short examples at the bottom of this readme).
As you'll get used to the library you'll probably want to start customizing the behaviour of your sinks or even exchange types of Record's properties. This library lets you do this. Just head over to docs page https://dnkpp.github.io/Simple-Log/ or have a look at /src/examples directory.
If you need an example for some advanced technics, don't hesitate asking me. As this library is growing I'll add more and more (hopefully useful) examples.

A friendly reminder at the end: This library is currently in an alpha state, where it may be possible that some API breaks will happen. If you need a fully stable library from now on, this is unfortunatly not what you're looking for. I'm sorry, but perhaps
it will be worth a second look in the near future.

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

### FetchContent
It is possible fetching this library via CMakes FetchContent module.

```cmake
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(<your_project_name>)

include(FetchContent)

FetchContent_Declare(
	simple_log
	GIT_REPOSITORY	https://github.com/DNKpp/Simple-Log
	GIT_TAG		origin/master
)
FetchContent_MakeAvailable(simple_log)

target_link_libraries(
	<your_target_name>
	PRIVATE simple_log
)
```

## Examples

### Easy Start
This is an example, which will print every message onto the console. Users will automatically receive a Core instance (gCore), Console Sink (gConsoleSink) and a Logger (gLog).
```cpp
/* With inclusion of this header, there will be automatically gCore, gConsoleSink and gLog constructed, which you might use.*/
#include <Simple-Log/ReadyToGo.hpp>

// just import everything into the current namespace
using namespace sl::log::ready_to_go;

int main()
{
	// This line will be printed on the console with the severity "info".
	gLog() << "Hello, World!";
	// You may adjust the severity for the currently created Record like so.
	gLog() << SetSev(SevLvl::debug) << "Mighty debug message";
	// The severity manipulator doesn't has to appear at front. Place it anywhere in your Record construction chain.
	gLog() << "Print my important hint!" << SetSev(SevLvl::hint);
}

/*Core will make sure, that all pending Records will be processed before it gets destructed.*/
/*
 * The above code may generate this output:
20:18:59.357 >>> info:: Hello, World!
20:18:59.357 >>> debug:: Mighty debug message
20:18:59.357 >>> hint:: Print my important hint!
 *
 * Keep in mind, you are completely free how you are going to format your message. This is just the default one.
 */
```

### Easy File logging
```cpp
/* With inclusion of this header, there will be automatically gCore, gConsoleSink and gLog constructed, which you might use.*/
#include <Simple-Log/ReadyToGo.hpp>

// just pull everything into the current namespace
using namespace sl::log::ready_to_go;

// this creates a new FileSink object, which will store all incoming messages in logfile.log
auto& gFileSink = gCore.makeSink<FileSink_t>("logfile.log");

int main()
{
	// Let our FileSink only handle important messages, e.g. warning and above
	gFileSink.setFilter(makeSeverityFilterFor<Record_t>(GreaterEquals{ SevLvl::warning }));

	// this message will only appear on the console
	gLog() << "Hello, World!";

	// while this message will also be saved in our logfile.log. Go ahead and see it yourself ;)
	gLog() << SetSev(SevLvl::warning) << "I'm an exemplary warning!";
}
```
