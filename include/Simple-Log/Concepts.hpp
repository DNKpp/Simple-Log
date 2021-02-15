//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_CONCEPTS_HPP
#define SL_LOG_CONCEPTS_HPP

#pragma once

#include <concepts>
#include <ostream>
#include <string>

namespace sl::log
{
	template <class T1>
	concept NotVoid = !std::is_same_v<T1, void>;

	template <class T>
	concept Record =
		std::movable<T> &&
		requires(T rec)
		{
			typename T::Message_t;
			typename T::TimePoint_t;
			typename T::SeverityLevel_t;
			typename T::Channel_t;
		} &&
		requires(const T& rec)
		{
			{ rec.message() } -> NotVoid;
			{ rec.severity() } -> NotVoid;
			{ rec.channel() } -> NotVoid;
			{ rec.timePoint() } -> NotVoid;
		} &&
		requires (T& rec)
		{
			{ rec.setMessage(std::declval<typename T::Message_t>()) };
			{ rec.setTimePoint(std::declval<typename T::TimePoint_t>()) };
			{ rec.setSeverity(std::declval<typename T::SeverityLevel_t>()) };
			{ rec.setChannel(std::declval<typename T::Channel_t>()) };
		};

	template <class T, class TRecord>
	concept RecordFormatterFor = Record<TRecord> && std::invocable<T, std::ostream&, const TRecord&>;

	template <class T, class TRecord>
	concept RecordFilterFor = Record<TRecord> && std::predicate<T, const TRecord&>;

	template <class T>
	concept FileStateHandler = std::invocable<T> && std::convertible_to<std::invoke_result_t<T>, std::string>;
	
}

#endif
