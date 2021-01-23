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
#include "Concepts.hpp"
#include "StringPattern.hpp"

namespace sl::log
{
	class FileSink :
		public BasicSink
	{
	public:
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

	private:
		StringPattern m_FileNamePattern;
		std::filesystem::path m_Directory;
		std::ofstream m_FileStream;
		std::optional<std::filesystem::path> m_CurrentFilePath;
		std::optional<std::size_t> m_MaxDirFileSize;

		void openFile()
		{
			closeFile();

			auto filePath = m_Directory;
			filePath.append(m_FileNamePattern.next());
			m_FileStream.open(filePath);
			m_CurrentFilePath = std::move(filePath);
		}

		void closeFile()
		{
			if (!m_FileStream.is_open())
			{
				return;
			}

			assert(m_CurrentFilePath && !std::empty(*m_CurrentFilePath));
			m_FileStream.close();

			removeFilesIfNecessary();
		}

		void removeFilesIfNecessary()
		{
			if (!m_MaxDirFileSize)
				return;
			// ToDo: use c++20 ranges::view
			auto directoryItr = std::filesystem::directory_iterator(*m_CurrentFilePath);

			std::vector<std::filesystem::directory_entry> files;
			for (const auto& entry : directoryItr)
			{
				if (is_regular_file(entry) && entry.path().extension() != m_CurrentFilePath->extension() &&
					!equivalent(entry.path(), *m_CurrentFilePath))
				{
					files.emplace_back(entry);
				}
			}

			std::ranges::sort(files, std::less(), [](const auto& file) { return last_write_time(file); });
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
	};
}

#endif
