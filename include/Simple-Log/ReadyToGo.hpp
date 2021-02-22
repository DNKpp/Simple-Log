//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_READY_TO_GO_HPP
#define SL_LOG_READY_TO_GO_HPP

#pragma once

#include "PresetTypes.hpp"

#include <iostream>

namespace sl::log::ready_to_go
{
	using namespace pre;

	inline Core_t gCore;
	inline auto& gConsoleSink{ gCore.makeSink<BasicSink_t>(std::cout) };
	inline auto gLog{ makeLogger<Logger_t>(gCore, SeverityLevel::info) };
}

#endif
