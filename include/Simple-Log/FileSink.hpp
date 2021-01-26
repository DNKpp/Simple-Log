//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SL_LOG_FILE_SINK_HPP
#define SL_LOG_FILE_SINK_HPP

#pragma once

#include <cassert>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <optional>

#include "BasicSink.hpp"
#include "StringPattern.hpp"

namespace sl::log
{
	/** \addtogroup sink
	 * @{
	 */
	
	class FileSink :
		public BasicSink
	{
		using Super = BasicSink;

	public:
		struct RotationRule
		{
			std::optional<std::size_t> fileSize;
			std::optional<std::chrono::seconds> duration;
		};

		struct CleanupRule
		{
			std::optional<std::size_t> directorySize;
			std::optional<std::size_t> fileCount;
		};

		FileSink(std::string fileNamePattern, std::filesystem::path directory = std::filesystem::current_path()) :
			BasicSink{ m_FileStream },
			m_FileNamePattern{ std::move(fileNamePattern) },
			m_Directory{ std::move(directory.remove_filename()) }
		{
		}

		~FileSink() noexcept
		{
			try
			{
				closeFile();
			}
			catch (...)
			{
			}
		}

		FileSink(const FileSink&) = delete;
		FileSink& operator =(const FileSink&) = delete;
		FileSink(FileSink&&) = delete;
		FileSink& operator =(FileSink&&) = delete;

		void setRotationRule(RotationRule rule) noexcept
		{
			std::scoped_lock lock{ m_RotationRuleMx };
			m_RotationRule = rule;
		}

		[[nodiscard]] RotationRule rotationRule() const noexcept
		{
			return load(m_RotationRule, m_RotationRuleMx);
		}

		void setCleanupRule(CleanupRule rule) noexcept
		{
			std::scoped_lock lock{ m_CleanupRuleMx };
			m_CleanupRule = rule;
		}

		[[nodiscard]] CleanupRule cleanupRule() const noexcept
		{
			return load(m_CleanupRule, m_CleanupRuleMx);
		}

		template <log::FileStateHandler THandler>
		void setOpeningHandler(THandler&& handler) noexcept
		{
			std::scoped_lock lock{ m_OpeningHandlerMx };
			m_OpeningHandler = std::forward<THandler>(handler);
		}

		void removeOpeningHandler() noexcept
		{
			std::scoped_lock lock{ m_OpeningHandlerMx };
			m_OpeningHandler = nullptr;
		}

		template <log::FileStateHandler THandler>
		void setClosingHandler(THandler&& handler) noexcept
		{
			std::scoped_lock lock{ m_ClosingHandlerMx };
			m_ClosingHandler = std::forward<THandler>(handler);
		}

		void removeClosingHandler() noexcept
		{
			std::scoped_lock lock{ m_ClosingHandlerMx };
			m_ClosingHandler = nullptr;
		}

		void log(const Record& rec) override
		{
			if (!m_FileStream.is_open())
			{
				openFile();
			}
			else if (shallRotate())
			{
				closeFile();
				openFile();
			}

			Super::log(rec);
		}

	private:
		using FileStateHandler = std::function<std::string()>;
		
		StringPattern m_FileNamePattern;
		std::filesystem::path m_Directory;
		std::ofstream m_FileStream;
		std::optional<std::filesystem::path> m_CurrentFilePath;

		// rotation related
		std::atomic<std::chrono::steady_clock::time_point> m_FileOpeningTime;
		mutable std::mutex m_RotationRuleMx;
		RotationRule m_RotationRule;

		// cleanup related
		mutable std::mutex m_CleanupRuleMx;
		CleanupRule m_CleanupRule;

		// File Handler
		std::mutex m_OpeningHandlerMx;
		FileStateHandler m_OpeningHandler;
		std::mutex m_ClosingHandlerMx;
		FileStateHandler m_ClosingHandler;

		void openFile()
		{
			auto filePath = m_Directory;
			filePath.append(m_FileNamePattern.next());
			m_FileStream.open(filePath);
			m_CurrentFilePath = std::move(filePath);
			m_FileOpeningTime = std::chrono::steady_clock::now();

			if (std::scoped_lock lock{ m_OpeningHandlerMx }; m_OpeningHandler)
			{
				writeToStream(m_OpeningHandler());
			}
		}

		void closeFile()
		{
			assert(m_FileStream.is_open() && "FileStream must be open.");
			assert(m_CurrentFilePath && !std::empty(*m_CurrentFilePath));

			if (std::scoped_lock lock{ m_ClosingHandlerMx }; m_ClosingHandler)
			{
				writeToStream(m_ClosingHandler());
			}
			m_FileStream.close();

			removeFilesIfNecessary();
		}

		void removeFilesIfNecessary()
		{
			// ToDo: use c++20 ranges::view
			auto directoryItr = std::filesystem::directory_iterator(m_CurrentFilePath->parent_path());

			std::vector<std::filesystem::directory_entry> files;
			for (const auto& entry : directoryItr)
			{
				if (is_regular_file(entry) && entry.path().extension() == m_CurrentFilePath->extension() &&
					!equivalent(entry.path(), *m_CurrentFilePath))
				{
					files.emplace_back(entry);
				}
			}

			std::ranges::sort(files, std::greater(), [](const auto& file) { return last_write_time(file); });
			fulfillFileCountCleanup(files);
			fulfillDirectorySizeCleanup(files);
		}

		template <class T, class TMutex>
		static T load(const T& object, TMutex& mutex)
		{
			std::scoped_lock lock{ mutex };
			return object;
		}

		void fulfillFileCountCleanup(std::vector<std::filesystem::directory_entry>& files) const
		{
			auto cleanupRule = load(m_CleanupRule, m_CleanupRuleMx);
			if (!cleanupRule.fileCount)
				return;
			
			while (*cleanupRule.fileCount < std::size(files))
			{
				auto& file = files.back();
				remove(file);
				files.pop_back();
			}
		}

		void fulfillDirectorySizeCleanup(std::vector<std::filesystem::directory_entry>& files) const
		{
			auto cleanupRule = load(m_CleanupRule, m_CleanupRuleMx);
			if (!cleanupRule.directorySize)
				return;

			auto size = std::accumulate(
										std::begin(files),
										std::end(files),
										0ull,
										[](auto value, const auto& file) { return value + file.file_size(); }
										);
			while (*cleanupRule.directorySize < size)
			{
				auto& file = files.back();
				size -= file_size(file);
				remove(file);
				files.pop_back();
			}
		}

		[[nodiscard]] bool shallRotate() const
		{
			assert(m_FileStream.is_open() && m_CurrentFilePath && !std::empty(*m_CurrentFilePath));

			auto rotationRule = load(m_RotationRule, m_RotationRuleMx);
			return (rotationRule.fileSize && *rotationRule.fileSize < file_size(*m_CurrentFilePath)) ||
				(rotationRule.duration && m_FileOpeningTime.load() + *rotationRule.duration < std::chrono::steady_clock::now());
		}
	};

	/** @}*/
}

#endif
