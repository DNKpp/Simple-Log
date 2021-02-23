//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

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
