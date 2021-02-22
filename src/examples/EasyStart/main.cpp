//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Log/ReadyToGo.hpp>

using namespace sl::log::ready_to_go;

int main()
{
	gLog() << "Hello, World!"; // This will print this message with the "info" severity
	// override default logger settings at the beginning
	gLog() << SetSev::debug << "Mighty debug message";
	// or at the end
	gLog() << "Print my important hint!" << SetSev::hint;
	// or in between of messages
	gLog() << "This" << " will " << SetSev::warning << " create " << " a " << " concatenated " << " warning " << " message";
	// and using default setup again
	gLog() << "Back to info";
}
