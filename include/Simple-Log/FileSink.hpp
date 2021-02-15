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

	/**
	 * \brief Class for logging into files 
	 * \details Instances of this class are linked to a specific file and writes every Record into it. Users can setup \ref Rotation "rotation" and cleanup rules.
	 *
	 * \section FileNamePattern File name pattern
	 * Users can provide a pattern string, which will be used for newly generated file names. Those file names will be generated when a new file shall be opened.
	 * There are mainly two parts of those strings:
	 * \li Constant parts are all substrings which do not contain any known tokens and will simply be used as is in file names.
	 * \li Dynamic parts are substrings which contain know tokens, which will be replaced with actual data when a new file name will be generated. Look at the \ref Tokens "token" section of this page for further details.
	 *
	 * \subsection Tokens
	 * Each token begins with \c %
	 * Token	| Effect
	 * ---------|--------
	 * \%Y		| year in 4 digits
	 * \%m		| month in 2 digits (including leading zeros)
	 * \%d		| day of month in 2 digits (including leading zeros)
	 * \%j		| day of year
	 * \%H		| hour in 24h format (2 digits with leading zeros)
	 * \%M		| minute (2 digits with leading zeros)
	 * \%S		| second (2 digits with leading zeros)
	 * \%N		| incrementing number starting at 1
	 * \%xN		| incrementing number with x digits and leading zeros. Replace \a x with an actual number (might be multiple digits). Starts at 1.
	 *
	 * \section Rotation Rotation
	 * Rotation rules are used as an indicator when to switch to a new file. Currently are two basic rules implemented:
	 * \li file size
	 * \li duration
	 *
	 * Each rule will be checked before a new Record will be written to file. If a new file shall be opened, a new file name will be generated via provided file name pattern.
	 * If this file already exists, it will be overriden (or an exception raises if it is already in use).
	 *
	 * \section Cleanup Cleanup
	 * While \ref Rotation "rotation" is used for indicating when to switch files, CleanupRule s are used for determining when to delete old files. Cleanup process will watch at the currently provided directory
	 * and will only observe files with equally file extensions as provided in the file name pattern. If no extension (e.g. \c .abcd) is used, all files without an extension will be observed.
	 *
	 * \section FileStateHandler File state handler
	 * These handler are used to generate messages on state changes for the internal managed file. Users can provide invokable objects, which will be called and shall return a std::string, which will then written synchronized
	 * to the std::fstream. The synchronization is the reason, why the user is not allowed to write to the stream themselves.\n
	 * Handlers shall have the following signature:
	 * \code{.cpp}
	 * std::string()
	 * \endcode
	 */
	template <Record TRecord>
	class FileSink :
		public BasicSink<TRecord>
	{
		using Super = BasicSink<TRecord>;

	public:
		using typename Super::Record_t;
		using typename Super::Formatter_t;
		using typename Super::Filter_t;

		/**
		 * \brief Type for configuring FileSink rotation rules
		 */
		struct RotationRule
		{
			std::optional<std::size_t> fileSize;
			std::optional<std::chrono::seconds> duration;
		};

		/**
		 * \brief Type for configuring FileSink cleanup rules
		 */
		struct CleanupRule
		{
			std::optional<std::size_t> directorySize;
			std::optional<std::size_t> fileCount;
		};

		/**
		 * \brief Constructor
		 * \details Constructs a new FileSink instance, which uses the provided file name pattern for newly opened files at the specified directory.
		 * \param fileNamePattern	Pattern string from which new file names will be generated.
		 * \param directory			The directory where all files of this sink will be generated.
		 */
		FileSink(std::string fileNamePattern, std::filesystem::path directory = std::filesystem::current_path()) :
			Super{ m_FileStream },
			m_FileNamePattern{ std::move(fileNamePattern) },
			m_Directory{ std::move(directory.remove_filename()) }
		{
		}

		/**
		 * \brief Destructor
		 * \details Finalizes and closes the current opened file (if any).
		 */
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

		/**
		 * \brief Deleted copy constructor
		 */
		FileSink(const FileSink&) = delete;
		/**
		 * \brief Deleted copy assign operator
		 */
		FileSink& operator =(const FileSink&) = delete;
		/**
		 * \brief Deleted move constructor
		 */
		FileSink(FileSink&&) = delete;
		/**
		 * \brief Deleted move assign operator
		 */
		FileSink& operator =(FileSink&&) = delete;

		/**
		 * \brief Applies a new RotationRule configuration.
		 * \param rule The new RotationRule configuration.
		 */
		void setRotationRule(RotationRule rule) noexcept
		{
			std::scoped_lock lock{ m_RotationRuleMx };
			m_RotationRule = rule;
		}

		/**
		 * \brief Returns a copy of the current RotationRule configuration
		 */
		[[nodiscard]] RotationRule rotationRule() const noexcept
		{
			return load(m_RotationRule, m_RotationRuleMx);
		}

		/**
		 * \brief Applies a new CleanupRule configuration.
		 * \param rule The new CleanupRule configuration.
		 */
		void setCleanupRule(CleanupRule rule) noexcept
		{
			std::scoped_lock lock{ m_CleanupRuleMx };
			m_CleanupRule = rule;
		}

		/**
		 * \brief Returns a copy of the current CleanupRule configuration
		 */
		[[nodiscard]] CleanupRule cleanupRule() const noexcept
		{
			return load(m_CleanupRule, m_CleanupRuleMx);
		}

		/**
		 * \brief Applies a new handler for opening files
		 * \details For further details look at \ref FileStateHandler "file state handler".
		 * \tparam THandler Type of handler (automatically deduced)
		 * \param handler invokable handler object
		 */
		template <FileStateHandler THandler>
		void setOpeningHandler(THandler&& handler) noexcept
		{
			std::scoped_lock lock{ m_OpeningHandlerMx };
			m_OpeningHandler = std::forward<THandler>(handler);
		}

		/**
		 * \brief Removes the active opening handler
		 */
		void removeOpeningHandler() noexcept
		{
			std::scoped_lock lock{ m_OpeningHandlerMx };
			m_OpeningHandler = nullptr;
		}

		/**
		 * \brief Applies a new handler for closing files
		 * \details For further details look at \ref FileStateHandler "file state handler".
		 * \tparam THandler Type of handler (automatically deduced)
		 * \param handler invokable handler object
		 */
		template <FileStateHandler THandler>
		void setClosingHandler(THandler&& handler) noexcept
		{
			std::scoped_lock lock{ m_ClosingHandlerMx };
			m_ClosingHandler = std::forward<THandler>(handler);
		}

		/**
		 * \brief Removes the active closing handler
		 */
		void removeClosingHandler() noexcept
		{
			std::scoped_lock lock{ m_ClosingHandlerMx };
			m_ClosingHandler = nullptr;
		}

		/**
		 * \brief Filters, formats and writes the passed record to the internal stream
		 * \details This function prints the passed record to the internal file. In forehand \ref Rotation "rotation" will be checked if a new file shall be opened. Internally uses to the
		 * BasicSink::log function.
		 * \param record Record object
		 */
		void log(const Record_t& record) override
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

			Super::log(record);
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
				Super::writeToStream(m_OpeningHandler());
			}
		}

		void closeFile()
		{
			assert(m_FileStream.is_open() && "FileStream must be open.");
			assert(m_CurrentFilePath && !std::empty(*m_CurrentFilePath));

			if (std::scoped_lock lock{ m_ClosingHandlerMx }; m_ClosingHandler)
			{
				Super::writeToStream(m_ClosingHandler());
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
