//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_STRING_PATTERN_HPP
#define SL_LOG_STRING_PATTERN_HPP

#pragma once

#include <algorithm>
#include <charconv>
#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>

namespace sl::log::detail
{
	struct IncNumberGenerator
	{
		std::size_t minWidth = 0;
		std::uint32_t current = 1;

		void operator()(std::ostream& stream)
		{
			stream << std::setfill('0') << std::setw(minWidth) << current++;
		}
	};

	// ToDo: modernize with c++20 chrono features
	struct DateTimeGenerator
	{
		std::string token;

		void operator()(std::ostream& stream) const
		{
			const auto now = std::chrono::system_clock::now();
			auto t_c = std::chrono::system_clock::to_time_t(now);
			stream << std::put_time(std::localtime(&t_c), token.c_str());
		}
	};

	struct StringGenerator
	{
		std::string_view str;

		void operator()(std::ostream& stream) const
		{
			stream << str;
		}
	};
}

namespace sl::log
{
	class StringPattern
	{
	public:
		StringPattern(std::string patternString) :
			m_PatternString{ std::move(patternString) },
			m_TokenGenerators{ makeTokenGeneratorsFromPatternString() }
		{
		}

		std::string next()
		{
			std::ostringstream ss;
			for (auto& token : m_TokenGenerators)
			{
				token(ss);
			}
			return std::move(ss).str();
		}

	private:
		std::string m_PatternString;

		using Generator = std::function<void(std::ostream&)>;
		std::vector<Generator> m_TokenGenerators;

		std::vector<Generator> makeTokenGeneratorsFromPatternString()
		{
			std::vector<Generator> generators;

			for (auto itr = std::begin(m_PatternString), end = std::end(m_PatternString); itr != end;)
			{
				auto tokenBegin = std::ranges::find(itr, end, '%');
				if (itr != tokenBegin)
				{
					generators.emplace_back(detail::StringGenerator{ { itr, tokenBegin } });
				}

				if (std::distance(tokenBegin, end) < 2)
				{
					break;
				}
				itr = tokenBegin + 1;

				switch (*(tokenBegin + 1))
				{
				case 'H': // hour (2 digits)
				case 'M': // minute (2 digits)
				case 'S': // second (2 digits)

				case 'Y': // year (4 digits)
				case 'm': // month (2 digits)
				case 'd': // day of month
				case 'j': // day of year
					generators.emplace_back(detail::DateTimeGenerator{ { tokenBegin, itr + 1 } });
					itr = tokenBegin + 2;
					break;

				case 'N': // IncNumber
					generators.emplace_back(detail::IncNumberGenerator{ .minWidth = 0 });
					itr = tokenBegin + 2;
					break;
				default: // IncNumber with custom width
					auto tokenEnd = std::ranges::find(tokenBegin + 1, end, 'N');
					// from_chars perform conversions until the first not convertible sign is detected, but in this case it should fail when there is any non-convertible sign
					// between %...N, thus I'll pre-check it here
					if (tokenEnd != end && std::ranges::all_of(
																tokenBegin + 1,
																tokenEnd,
																[](char c) { return '0' <= c && c <= '9'; }
															))
					{
						std::size_t width = 0;
						if (auto [p, ec] = std::from_chars(&*(tokenBegin + 1), &*tokenEnd, width); ec == std::errc())
						{
							generators.emplace_back(detail::IncNumberGenerator{ .minWidth = width });
							itr = tokenEnd + 1;
						}
					}
				}
			}
			return generators;
		}
	};
}

#endif
