//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/* With inclusion of this header, there will be automatically gCore, gConsoleSink and gLog constructed, which you might use.*/
#include <Simple-Log/ReadyToGo.hpp>

// just pull everything into the current namespace
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
