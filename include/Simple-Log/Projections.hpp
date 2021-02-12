//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_PROJECTIONS_HPP
#define SL_LOG_PROJECTIONS_HPP

#pragma once

#include <any>
#include <functional>

#include "Record.hpp"

namespace sl::log
{
	template <class TProjection, class TTransform>
	class TransformedProjection
	{
	public:
		explicit TransformedProjection(TProjection projection, TTransform transform) :
			m_Projection{ std::move(projection) },
			m_Transform{ std::move(transform) }
		{
		}

		auto operator ()(const Record& rec)
		{
			return std::invoke(m_Transform, std::invoke(m_Projection, rec));
		}

	private:
		TProjection m_Projection;
		TTransform m_Transform;
	};

	template <class T>
	struct AnyTransformer
	{
		const T* operator ()(const std::any& any) const noexcept
		{
			return std::any_cast<T>(&any);
		}
	};

	template <class TSeverityType>
	inline TransformedProjection severityTransProjection{ &Record::severity, AnyTransformer<TSeverityType>{} };

	template <class TChannelType>
	inline TransformedProjection channelTransProjection{ &Record::channel, AnyTransformer<TChannelType>{} };

	template <class TUserDataType>
	inline TransformedProjection userDataTransProjection{ &Record::severity, AnyTransformer<TUserDataType>{} };

	struct DeducePointer
	{
		template <Pointer T>
		auto& operator ()(T obj) const noexcept
		{
			return *obj;
		}
	};
}

#endif
