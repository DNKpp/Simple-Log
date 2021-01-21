//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_ISINK_HPP
#define SL_LOG_ISINK_HPP

#pragma once

#include "Record.hpp"

namespace sl::log
{	
	class ISink
	{
	public:
		virtual ~ISink() noexcept = default;

		virtual void log(const Record& record) = 0;
	};
}

#endif
