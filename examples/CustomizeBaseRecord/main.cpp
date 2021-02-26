//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Log/PresetTypes.hpp>
#include <Simple-Log/Simple-Log.hpp>

#include <sstream>

using namespace sl::log;
using preset::SevLvl;

enum class Channel
{
	standard,
	network,
	stats
};

// A simple overload, thus our Channel enum can be printed into every ostream object.
inline std::ostream& operator <<(std::ostream& out, Channel lvl)
{
	constexpr const char* str[] = { "standard", "network", "stats" };
	out << str[static_cast<std::size_t>(lvl)];
	return out;
}

// Yes, these are a few typedefs which might seem tedious at the first glance, but you'll usually need to do this once per program.
using Record_t = BaseRecord<SevLvl, Channel>;
using Core_t = Core<Record_t>;
using ConsoleSink_t = ConsoleSink<Record_t>;
using FileSink_t = FileSink<Record_t>;
using Logger_t = BaseLogger<Record_t>;

inline Core_t gCore;
// every Record, which will be created by this Logger will be from Channel "standard" by default 
inline auto gLog = makeLogger<Logger_t>(gCore, SevLvl::info, Channel::standard);
inline auto& gConsoleSink
{
	[]() -> auto&
	{
		// let's create the console sink in disabled state. Will become automatically enabled after this scope is left.
		auto wrappedSink = gCore.makeDisabledSink<ConsoleSink_t>();
		// Only messages with the Channel network shall be printed onto the console
		wrappedSink->setFilter(makeChannelFilterFor<Record_t>(Equals{ Channel::network }));
		// setting up a custom formatter, thus for each Record only the channel followed by the message will be printed.
		wrappedSink->setFormatter(
								[](const Record_t& record)
								{
									std::stringstream out;
									out << record.channel() << " >> " << record.message();
									return std::move(out).str();
								}
								);

		return *wrappedSink;
	}()
};

int main()
{
	gLog() << "Hello, World!";	// this message is ignored by our Console sink
	gLog() << SetChan(Channel::network) << "Hello, Network!"; // our Console sink handles this record.
}

/* The output of the console looks like this:
 network >> Hello, Network!

 */
