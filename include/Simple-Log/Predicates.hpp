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

namespace sl::log::pred
{
	template <std::equality_comparable T>
	class EqualsToConstant
	{
	public:
		explicit EqualsToConstant(T to) :
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

	template <std::equality_comparable T>
	class NotEqualsToConstant
	{
	public:
		explicit NotEqualsToConstant(T to) :
			m_To{ std::move(to) }
		{
		}

		bool operator ()(const T& other) const
		{
			return other != m_To;
		}

	private:
		T m_To;
	};

	template <std::totally_ordered T>
	class LessToConstant
	{
	public:
		explicit LessToConstant(T to) :
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
	class GreaterToConstant
	{
	public:
		explicit GreaterToConstant(T to) :
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
	class LessEqualsToConstant
	{
	public:
		explicit LessEqualsToConstant(T to) :
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
	class GreaterEqualsToConstant
	{
	public:
		explicit GreaterEqualsToConstant(T to) :
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
	class BetweenConstants
	{
	public:
		explicit BetweenConstants(T one, T two) :
			m_Low{ std::min(one, two) },
			m_High{ std::max(two, one) }
		{
		}

		bool operator ()(const T& other) const
		{
			return m_Low <= other && other <= m_High;
		}

	private:
		T m_Low;
		T m_High;
	};

	inline const NotEqualsToConstant<const void*> notNullptr{ nullptr };
}

#endif
