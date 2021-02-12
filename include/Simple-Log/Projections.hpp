//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_PROJECTIONS_HPP
#define SL_LOG_PROJECTIONS_HPP

#pragma once

#include <any>
#include <functional>

#include "Concepts.hpp"
#include "Record.hpp"

namespace sl::log::proj
{
	template <class TProjection, class TTransform>
	class Transformer
	{
	public:
		explicit Transformer(TProjection projection, TTransform transform) :
			m_Projection{ std::move(projection) },
			m_Transform{ std::move(transform) }
		{
		}

		auto operator ()(const Record& rec) const
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
	inline const Transformer severityCast{ &Record::severity, AnyTransformer<TSeverityType>{} };

	template <class TChannelType>
	inline const Transformer channelCast{ &Record::channel, AnyTransformer<TChannelType>{} };

	template <class TUserDataType>
	inline const Transformer userDataCast{ &Record::severity, AnyTransformer<TUserDataType>{} };

	struct
	{
		template <Pointer T>
		auto& operator ()(T obj) const noexcept
		{
			return *obj;
		}
	} inline const deducePointer;
}

#endif
