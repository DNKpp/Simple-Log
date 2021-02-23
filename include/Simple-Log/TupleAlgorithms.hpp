//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_TUPLE_ALGORITHMS_HPP
#define SL_LOG_TUPLE_ALGORITHMS_HPP

#pragma once

#include <functional>
#include <tuple>

namespace sl::log::detail
{
	class TupleAllOf
	{
	public:
		template <class TTuple, class... TArgs>
		constexpr bool operator ()(TTuple& tuple, TArgs&&... args) const
		{
			return invoke<0>(tuple, std::forward<TArgs>(args)...);
		}

	private:
		template <std::size_t index, class TTuple, class... TArgs>
		constexpr bool invoke(TTuple& tuple, TArgs&&... args) const
		{
			if constexpr (index < std::tuple_size_v<TTuple>)
			{
				if (!std::invoke(std::get<index>(tuple), std::forward<TArgs>(args)...))
				{
					return false;
				}
				return invoke<index + 1>(tuple, std::forward<TArgs>(args)...);
			}
			return true;
		}
	};

	class TupleAnyOf
	{
	public:
		template <class TTuple, class... TArgs>
		constexpr bool operator ()(TTuple& tuple, TArgs&&... args) const
		{
			return invoke<0>(tuple, std::forward<TArgs>(args)...);
		}

	private:
		template <std::size_t index, class TTuple, class... TArgs>
		constexpr bool invoke(TTuple& tuple, TArgs&&... args) const
		{
			if constexpr (index < std::tuple_size_v<TTuple>)
			{
				if (std::invoke(std::get<index>(tuple), std::forward<TArgs>(args)...))
				{
					return true;
				}
				return invoke<index + 1>(tuple, std::forward<TArgs>(args)...);
			}
			return false;
		}
	};

	class TupleNoneOf :
		private TupleAnyOf
	{
		using Super = TupleAnyOf;

	public:
		template <class TTuple, class... TArgs>
		constexpr bool operator ()(TTuple& tuple, TArgs&&... args) const
		{
			return !Super::operator()(tuple, std::forward<TArgs>(args)...);
		}
	};

	class TupleForEach
	{
	public:
		template <class TTuple, class TFunc>
		constexpr void operator ()(TTuple& tuple, TFunc func) const
		{
			invoke<0>(tuple, func);
		}

	private:
		template <std::size_t index, class TTuple, class TFunc>
		constexpr void invoke(TTuple& tuple, TFunc func) const
		{
			if constexpr (index < std::tuple_size_v<TTuple>)
			{
				std::invoke(func, std::get<index>(tuple));
				invoke<index + 1>(tuple, func);
			}
		}
	};
}

#endif
