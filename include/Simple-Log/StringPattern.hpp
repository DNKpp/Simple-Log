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
#include <iomanip>
#include <regex>
#include <sstream>
#include <variant>

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

	using Generator = std::variant<StringGenerator, IncNumberGenerator, DateTimeGenerator>;

	[[nodiscard]] inline Generator makeGeneratorFromMatch(std::string_view token)
	{
		if (token.starts_with('%'))
		{
			// ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement
			switch (token.back())
			{
			case 'H': // hour (2 digits)
			case 'M': // minute (2 digits)
			case 'S': // second (2 digits)

			case 'Y': // year (4 digits)
			case 'm': // month (2 digits)
			case 'd': // day of month
			case 'j': // day of year
				return DateTimeGenerator{ { std::cbegin(token), std::cbegin(token) + 2 } };

			case 'N':
				token.remove_prefix(1);
				std::size_t width = 0;
				std::from_chars(token.data(), &token.back(), width);
				return IncNumberGenerator{ .minWidth = width };
			}
		}

		// treat everything else as const substring, even if it contains a %
		return StringGenerator{ token };
	}

	[[nodiscard]] inline std::vector<Generator> makeTokenGeneratorsFromPatternString(std::string_view patternString)
	{
		std::vector<Generator> generators;

		const std::regex regEx{ "%(Y|m|d|H|M|S|\\d*N)" };
		std::transform(
						std::cregex_token_iterator{
							patternString.data(),
							patternString.data() + std::size(patternString),
							regEx,
							{ -1, 0 }
						},
						std::cregex_token_iterator{},
						std::back_inserter(generators),
						[](const auto& match)
						{
							return makeGeneratorFromMatch({ match.first, match.second });
						}
					);
		return generators;
	}
}

namespace sl::log
{
	/**
	 * \brief Helper class for generating patterned strings
	 * \details Tokenize the pattern string and creates generators, which will then create the actual substrings on demand.
	 */
	class StringPattern
	{
	public:
		/**
		 * \brief Constructor
		 * \param patternString Pattern
		 */
		explicit StringPattern(std::string patternString) :
			m_PatternString{ std::move(patternString) },
			m_TokenGenerators{ detail::makeTokenGeneratorsFromPatternString(m_PatternString) }
		{
		}

		/**
		 * \brief Creates a new string
		 * \details The returned string will be created on demand and follows the pattern string rules.
		 * \return Generated string.
		 */
		std::string next()
		{
			std::ostringstream ss;
			for (auto& token : m_TokenGenerators)
			{
				std::visit(
							[&ss](auto& generator)
							{
								generator(ss);
							},
							token
						);
			}
			return std::move(ss).str();
		}

	private:
		std::string m_PatternString;
		std::vector<detail::Generator> m_TokenGenerators;
	};
}

#endif
