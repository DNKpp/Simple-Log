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

#include "Predicates.hpp"
#include "Projections.hpp"
#include "Record.hpp"

namespace sl::log
{
	template <class TProjection, std::predicate<std::invoke_result_t<TProjection, const Record&>> TUnaryPredicate>
	class Filter
	{
	public:
		Filter(TProjection projection, TUnaryPredicate predicate) :
			m_Predicate(std::move(predicate)),
			m_Projection(std::move(projection))
		{
		}

		bool operator ()(const Record& rec)
		{
			return std::invoke(m_Predicate, std::invoke(m_Projection, rec));
		}

	private:
		TUnaryPredicate m_Predicate;
		TProjection m_Projection;
	};

	template <class TProjection1,
		std::predicate<std::invoke_result_t<TProjection1, const Record&>> TUnaryPredicate1,
		std::invocable<std::invoke_result_t<TProjection1, const Record&>> TProjection2,
		std::predicate<std::invoke_result_t<TProjection2, std::invoke_result_t<TProjection1, const Record&>>> TUnaryPredicate2
	>
	class Filter2
	{
	public:
		Filter2(TProjection1 projection1, TUnaryPredicate1 predicate1, TProjection2 projection2, TUnaryPredicate2 predicate2) :
			m_Predicate1(std::move(predicate1)),
			m_Projection1(std::move(projection1)),
			m_Predicate2(std::move(predicate2)),
			m_Projection2(std::move(projection2))
		{
		}

		bool operator ()(const Record& rec)
		{
			if (auto projected1 = std::invoke(m_Projection1, rec); std::invoke(m_Predicate1, projected1))
			{
				return std::invoke(m_Predicate2, std::invoke(m_Projection2, projected1));
			}
			return false;
		}

	private:
		TUnaryPredicate1 m_Predicate1;
		TProjection1 m_Projection1;
		TUnaryPredicate2 m_Predicate2;
		TProjection2 m_Projection2;
	};

	template <class TChannel, std::predicate<TChannel> TUnaryPredicate>
	auto makeChannelFilter(TUnaryPredicate&& predicate)
	{
		return Filter2{
			channelTransProjection<TChannel>,
			NotNullptr{},
			DeducePointer{},
			std::forward<TUnaryPredicate>(predicate)
		};
	}

	template <class TSeverity, std::predicate<TSeverity> TUnaryPredicate>
	auto makeSeverityFilter(TUnaryPredicate&& predicate)
	{
		return Filter2{
			severityTransProjection<TSeverity>,
			NotNullptr{},
			DeducePointer{},
			std::forward<TUnaryPredicate>(predicate)
		};
	}

	template <class TUserData, std::predicate<TUserData> TUnaryPredicate>
	auto makeUserDataFilter(TUnaryPredicate&& predicate)
	{
		return Filter2{
			userDataTransProjection<TUserData>,
			NotNullptr{},
			DeducePointer{},
			std::forward<TUnaryPredicate>(predicate)
		};
	}

	struct FilterAllOf
	{
		template <std::ranges::input_range TRange>
		bool operator ()(const TRange& range, const Record& rec) const
		{
			return std::ranges::all_of(
										range,
										[&](const auto& filter)
										{
											return std::invoke(filter, rec);
										}
									);
		}
	};

	struct FilterAnyOf
	{
		template <std::ranges::input_range TRange>
		bool operator ()(const TRange& range, const Record& rec) const
		{
			return std::ranges::any_of(
										range,
										[&](const auto& filter)
										{
											return std::invoke(filter, rec);
										}
									);
		}
	};

	struct FilterNoneOf
	{
		template <std::ranges::input_range TRange>
		bool operator ()(const TRange& range, const Record& rec) const
		{
			return std::ranges::none_of(
										range,
										[&](const auto& filter)
										{
											return std::invoke(filter, rec);
										}
										);
		}
	};

	template <class TAlgorithm, std::predicate<const Record&>... TFilter>
	class FilterChain
	{
	public:
		explicit FilterChain(TFilter ...filter) :
			FilterChain(TAlgorithm{}, std::forward<TFilter>(filter)...)
		{
		}

		explicit FilterChain(TAlgorithm algorithm, TFilter&& ...filter) :
			m_Algorithm(std::move(algorithm)),
			m_Filter{ std::forward<TFilter>(filter)... }
		{
		}

		bool operator()(const Record& rec)
		{
			return std::invoke(m_Algorithm, m_Filter, rec);
		}

	private:
		TAlgorithm m_Algorithm;
		// Yes, I could use a tuple, but then I must implement all_of and none_of algorithms myself.
		std::array<std::function<bool(const Record&)>, sizeof...(TFilter)> m_Filter;
	};

	template <std::predicate<const Record&>... TFilter>
	using FilterDisjunction = FilterChain<FilterAllOf, TFilter...>;

	template <std::predicate<const Record&>... TFilter>
	using FilterConjunction = FilterChain<FilterAnyOf, TFilter...>;
}

#endif
