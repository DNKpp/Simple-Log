//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_CONCEPTS_HPP
#define SL_LOG_CONCEPTS_HPP

#pragma once

#include <concepts>
#include <ostream>

namespace sl::log
{
	class Record;

	template <class T>
	concept RecordFormatter = (std::movable<T> || std::copyable<T>) && std::invocable<T, std::ostream&, const Record&>;

	template <class T>
	concept RecordFilter = (std::movable<T> || std::copyable<T>) && std::predicate<T, const Record&>;

	template <class T>
	concept FileStateHandler = (std::movable<T> || std::copyable<T>) && std::invocable<T, std::ostream&, const Record&>;
	
}

#endif
