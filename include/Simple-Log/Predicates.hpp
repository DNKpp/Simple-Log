//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_PREDICATES_HPP
#define SL_LOG_PREDICATES_HPP

#pragma once

#include <algorithm>
#include <concepts>

namespace sl::log
{
	/** \addtogroup Predicates Predicates
	 * @{
	 */

	/**
	 * \brief Compares equality with constant at invocation
	 */
	template <std::equality_comparable T>
	class Equals
	{
	public:
		constexpr explicit Equals(T to) :
			m_To{ std::move(to) }
		{
		}

		template <std::equality_comparable_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return other == m_To;
		}

	private:
		T m_To;
	};

	/**
	 * \brief Compares non-equality with constant at invocation
	 */
	template <std::equality_comparable T>
	class NotEquals
	{
	public:
		constexpr explicit NotEquals(T to) :
			m_To{ std::move(to) }
		{
		}

		template <std::equality_comparable_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return other != m_To;
		}

	private:
		T m_To;
	};

	/**
	 * \brief Compares less-ordering with constant at invocation
	 */
	template <std::totally_ordered T>
	class Less
	{
	public:
		constexpr explicit Less(T to) :
			m_To{ std::move(to) }
		{
		}

		template <std::totally_ordered_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return other < m_To;
		}

	private:
		T m_To;
	};

	/**
	 * \brief Compares greater-ordering with constant at invocation
	 */
	template <std::totally_ordered T>
	class Greater
	{
	public:
		constexpr explicit Greater(T to) :
			m_To{ std::move(to) }
		{
		}

		template <std::totally_ordered_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return other > m_To;
		}

	private:
		T m_To;
	};

	/**
	 * \brief Compares less-equality-ordering with constant at invocation
	 */
	template <std::totally_ordered T>
	class LessEquals
	{
	public:
		constexpr explicit LessEquals(T to) :
			m_To{ std::move(to) }
		{
		}

		template <std::totally_ordered_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return other <= m_To;
		}

	private:
		T m_To;
	};

	/**
	 * \brief Compares greater-equality-ordering with constant at invocation
	 */
	template <std::totally_ordered T>
	class GreaterEquals
	{
	public:
		constexpr explicit GreaterEquals(T to) :
			m_To{ std::move(to) }
		{
		}

		template <std::totally_ordered_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return other >= m_To;
		}

	private:
		T m_To;
	};

	/**
	 * \brief Compares less-ordering with high and greater-ordering with low constant at invocation
	 */
	template <std::totally_ordered T>
	class Between
	{
	public:
		constexpr explicit Between(T one, T two) :
			m_Low{ std::min(one, two) },
			m_High{ std::max(two, one) }
		{
		}

		template <std::totally_ordered_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return m_Low < other && other < m_High;
		}

	private:
		T m_Low;
		T m_High;
	};

	/**
	 * \brief Compares less-equality-ordering with high and greater-equality-ordering with low constant at invocation
	 */
	template <std::totally_ordered T>
	class BetweenEquals
	{
	public:
		constexpr explicit BetweenEquals(T one, T two) :
			m_Low{ std::min(one, two) },
			m_High{ std::max(two, one) }
		{
		}

		template <std::totally_ordered_with<T> U>
		constexpr bool operator ()(const U& other) const
		{
			return m_Low <= other && other <= m_High;
		}

	private:
		T m_Low;
		T m_High;
	};

	/** @}*/
}

#endif
