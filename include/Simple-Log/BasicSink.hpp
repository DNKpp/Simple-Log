//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_BASIC_SINK_HPP
#define SL_LOG_BASIC_SINK_HPP

#pragma once

#include <ostream>

#include "ISink.hpp"

namespace sl::log
{
	class BasicSink :
		public ISink
	{
	public:
		BasicSink(std::ostream& stream) :
			m_Stream{ stream }
		{
		}

		~BasicSink() noexcept = default;

		BasicSink(const BasicSink&) = delete;
		BasicSink& operator =(const BasicSink&) = delete;

		BasicSink(BasicSink&&) = delete;
		BasicSink& operator =(BasicSink&&) = delete;
		
		void log(const Record& record) override
		{
			m_Stream << record.message << std::endl;
		}

	private:
		std::ostream& m_Stream;
	};
}

#endif
