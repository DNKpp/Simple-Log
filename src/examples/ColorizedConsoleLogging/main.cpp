//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Log/PresetTypes.hpp>

// just pull everything into the current namespace
using namespace sl::log::preset;
using namespace sl::log;

int main()
{
	Core_t core;
	auto log = makeLogger<Logger_t>(core, SevLvl::info);
	
	auto& gConsoleSink = core.makeSink<ConsoleSink_t>();

	using Color = ConsoleTextStyle::Color;
	using Style = ConsoleTextStyle::Style;
	gConsoleSink.setTextStylePolicy(
		makeConsoleTextStyleTableFor<Record_t>(
			&Record_t::severity,	// use the severity property
			{
				// the desired SevLvl gets linked to a color and style setup
				{ SevLvl::debug,	{ .textColor = Color::yellow, .bgColor = Color::gray } },
				{ SevLvl::hint,		{ .bgColor = Color::green } },
				{ SevLvl::warning,	{ .bgColor = Color::yellow } },
				{ SevLvl::error,	{ .style = Style::bold,		.textColor = Color::red,	.bgColor = Color::yellow } },
				{ SevLvl::fatal,	{ .style = Style::crossed,	.textColor = Color::gray,	.bgColor = Color::red } }
			}
		)
	);
		
	log() << "Hello, World!";
	log() << SetSev(SevLvl::debug) << "Mighty debug message";
	log() << "Print as hint!" << SetSev(SevLvl::hint);
	log() << SetSev(SevLvl::warning) << "be warned...";
	log() << "Print my important error!" << SetSev(SevLvl::error);
	log() << "Print it fatal!" << SetSev(SevLvl::fatal);
}
// Above lines will be printed nicely colorful onto the console. Go, test it out ;)
