//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_PREDICATES_HPP
#define SL_LOG_PREDICATES_HPP

#pragma once

#include <algorithm>
#include <concepts>

#include "Concepts.hpp"

namespace sl::log
{
	template <std::equality_comparable T>
	class Equals
	{
	public:
		explicit Equals(T to) :
			m_To{ std::move(to) }
		{
		}

		bool operator ()(const T& other) const
		{
			return other == m_To;
		}

	private:
		T m_To;
	};

	template <std::totally_ordered T>
	class Less
	{
	public:
		explicit Less(T to) :
			m_To{ std::move(to) }
		{
		}

		bool operator ()(const T& other) const
		{
			return other < m_To;
		}

	private:
		T m_To;
	};

	template <std::totally_ordered T>
	class Greater
	{
	public:
		explicit Greater(T to) :
			m_To{ std::move(to) }
		{
		}

		bool operator ()(const T& other) const
		{
			return other > m_To;
		}

	private:
		T m_To;
	};

	template <std::totally_ordered T>
	class LessEquals
	{
	public:
		explicit LessEquals(T to) :
			m_To{ std::move(to) }
		{
		}

		bool operator ()(const T& other) const
		{
			return other <= m_To;
		}

	private:
		T m_To;
	};

	template <std::totally_ordered T>
	class GreaterEquals
	{
	public:
		explicit GreaterEquals(T to) :
			m_To{ std::move(to) }
		{
		}

		bool operator ()(const T& other) const
		{
			return other >= m_To;
		}

	private:
		T m_To;
	};

	template <std::totally_ordered T>
	class Between
	{
	public:
		explicit Between(T one, T two)
		{
			auto [min, max] = std::minmax(one, two);
			m_Low = min;
			m_High = max;
		}

		bool operator ()(const T& other) const
		{
			return m_Low <= other && other <= m_High;
		}

	private:
		T m_Low;
		T m_High;
	};

	struct NotNullptr
	{
		template <Pointer T>
		bool operator ()(T obj) const noexcept
		{
			return obj != nullptr;
		}
	};
}

#endif
