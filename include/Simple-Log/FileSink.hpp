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
		
		FileSink(std::string fileNamePattern, std::filesystem::path directory = std::filesystem::current_path()) :
			BasicSink{ m_FileStream },
			m_FileNamePattern{ std::move(fileNamePattern) },
			m_Directory{ std::move(directory.remove_filename()) }
		{
			openFile();
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
			m_RotationRule = rule;
		}

		void log(const Record& rec) override
		{
			if (shallRotate())
			{
				closeFile();
				openFile();
			}
			
			Super::log(rec);
		}

		void setMaxDirectorySize(std::optional<std::size_t> size) noexcept
		{
			m_MaxDirFileSize = size;
		}

		[[nodiscard]] const std::optional<std::size_t>& maxDirectorySize() const noexcept
		{
			return m_MaxDirFileSize;
		}

	private:
		StringPattern m_FileNamePattern;
		std::filesystem::path m_Directory;
		std::ofstream m_FileStream;
		std::optional<std::filesystem::path> m_CurrentFilePath;
		std::optional<std::size_t> m_MaxDirFileSize;
		std::chrono::steady_clock::time_point m_FileOpeningTime;
		RotationRule m_RotationRule;

		void openFile()
		{
			auto filePath = m_Directory;
			filePath.append(m_FileNamePattern.next());
			m_FileStream.open(filePath);
			m_CurrentFilePath = std::move(filePath);
			m_FileOpeningTime = std::chrono::steady_clock::now();
		}

		void closeFile()
		{
			assert(m_FileStream.is_open() && "FileStream must be open.");
			assert(m_CurrentFilePath && !std::empty(*m_CurrentFilePath));
			m_FileStream.close();

			removeFilesIfNecessary();
		}

		void removeFilesIfNecessary()
		{
			if (!m_MaxDirFileSize)
				return;
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
			auto size = std::accumulate(
										std::begin(files),
										std::end(files),
										0ull,
										[](auto value, const auto& file) { return value + file.file_size(); }
										);
			while (*m_MaxDirFileSize < size)
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
			
			return m_RotationRule.fileSize && *m_RotationRule.fileSize < file_size(*m_CurrentFilePath) ||
				m_RotationRule.duration && m_FileOpeningTime + *m_RotationRule.duration < std::chrono::steady_clock::now();
		}
	};
}

#endif
