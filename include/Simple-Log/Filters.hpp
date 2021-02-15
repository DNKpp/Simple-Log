//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_FILTERS_HPP
#define SL_LOG_FILTERS_HPP

#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <functional>

#include "Concepts.hpp"

namespace sl::log::detail
{
	class TupleAllOf
	{
	public:
		template <class TTuple, Record TRecord>
		constexpr bool operator ()(TTuple& tuple, const TRecord& record) const
		{
			return invoke<0>(tuple, record);
		}

	private:
		template <std::size_t Index, class TTuple, Record TRecord>
		constexpr bool invoke(TTuple& tuple, const TRecord& record) const
		{
			if constexpr (Index < std::tuple_size_v<TTuple>)
			{
				if (!std::invoke(std::get<Index>(tuple), record))
				{
					return false;
				}
				return invoke<Index + 1>(tuple, record);
			}
			return true;
		}
	};

	class TupleAnyOf
	{
	public:
		template <class TTuple, Record TRecord>
		constexpr bool operator ()(TTuple& tuple, const TRecord& record) const
		{
			return invoke<0>(tuple, record);
		}

	private:
		template <std::size_t Index, class TTuple, Record TRecord>
		constexpr bool invoke(TTuple& tuple, const TRecord& record) const
		{
			if constexpr (Index < std::tuple_size_v<TTuple>)
			{
				if (std::invoke(std::get<Index>(tuple), record))
				{
					return true;
				}
				return invoke<Index + 1>(tuple, record);
			}
			return false;
		}
	};

	class TupleNoneOf :
		private TupleAnyOf
	{
		using Super = TupleAnyOf;

	public:
		template <class TTuple, Record TRecord>
		constexpr bool operator ()(TTuple& tuple, const TRecord& record) const
		{
			return !Super::operator()(tuple, record);
		}
	};
}

namespace sl::log
{
	template <class TProjection, class TUnaryPredicate>
	class ProjectionFilter
	{
	public:
		using Projection_t = TProjection;
		using UnaryPredicate_t = TUnaryPredicate;

		constexpr ProjectionFilter(TProjection projection, TUnaryPredicate predicate) :
			m_Projection(std::move(projection)),
			m_Predicate(std::move(predicate))
		{
		}

		template <Record TRecord>
		constexpr bool operator ()(const TRecord& rec)
		{
			return std::invoke(m_Predicate, std::invoke(m_Projection, rec));
		}

	private:
		Projection_t m_Projection;
		UnaryPredicate_t m_Predicate;
	};

	template <class TAlgorithm, class... TFilter>
	class FilterChain
	{
	public:
		constexpr explicit FilterChain(TFilter ...filter) :
			m_Algorithm{},
			m_Filter{ std::move(filter)... }
		{
		}

		constexpr explicit FilterChain(TAlgorithm algorithm, TFilter ...filter) :
			m_Algorithm(std::move(algorithm)),
			m_Filter{ std::forward<TFilter>(filter)... }
		{
		}

		template <Record TRecord>
		constexpr bool operator()(const TRecord& rec)
		{
			return std::invoke(m_Algorithm, m_Filter, rec);
		}

	private:
		TAlgorithm m_Algorithm;
		std::tuple<TFilter...> m_Filter;
	};

	//ToDo: Clang currently doesn't support alias CTAD: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1814r0.html
	//template <std::predicate<const Record&>... TFilter>
	//using FilterDisjunction = FilterChain<FilterAllOf, TFilter...>;
	// 
	//template <std::predicate<const Record&>... TFilter>
	//using FilterConjunction = FilterChain<FilterAnyOf, TFilter...>;

	template <class... TFilter>
	class FilterAllOf :
		public FilterChain<detail::TupleAnyOf, TFilter...>
	{
		using Algorithm = detail::TupleAnyOf;

	public:
		constexpr explicit FilterAllOf(TFilter ... filter) :
			FilterChain<Algorithm, TFilter...>{ std::move(filter)... }
		{
		}
	};

	template <class... TFilter>
	class FilterAnyOf :
		public FilterChain<detail::TupleAnyOf, TFilter...>
	{
		using Algorithm = detail::TupleAnyOf;

	public:
		constexpr explicit FilterAnyOf(TFilter ... filter) :
			FilterChain<Algorithm, TFilter...>{ std::move(filter)... }
		{
		}
	};

	template <class... TFilter>
	class FilterNoneOf :
		public FilterChain<detail::TupleNoneOf, TFilter...>
	{
		using Algorithm = detail::TupleNoneOf;

	public:
		constexpr explicit FilterNoneOf(TFilter ... filter) :
			FilterChain<Algorithm, TFilter...>{ std::move(filter)... }
		{
		}
	};

	template <Record TRecord, std::predicate<const typename TRecord::Message_t&> TUnaryPredicate>
	constexpr auto makeMessageFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::message, std::forward<TUnaryPredicate>(predicate) };
	}

	template <Record TRecord, std::predicate<const typename TRecord::SeverityLevel_t&> TUnaryPredicate>
	constexpr auto makeSeverityFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::severity, std::forward<TUnaryPredicate>(predicate) };
	}

	template <Record TRecord, std::predicate<const typename TRecord::Channel_t&> TUnaryPredicate>
	constexpr auto makeChannelFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::channel, std::forward<TUnaryPredicate>(predicate) };
	}

	template <Record TRecord, std::predicate<const typename TRecord::TimePoint_t&> TUnaryPredicate>
	constexpr auto makeTimePointFilterFor(TUnaryPredicate&& predicate)
	{
		return ProjectionFilter{ &TRecord::timePoint, std::forward<TUnaryPredicate>(predicate) };
	}
}

#endif
