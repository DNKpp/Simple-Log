//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <Simple-Log/PresetTypes.hpp>
#include <Simple-Log/Simple-Log.hpp>

#include <iostream>
#include <sstream>

using namespace sl::log;
using preset::SevLvl;

class MyCustomRecord :
	public preset::Record_t
{
public:
	using Domain_t = std::string;
	
	Domain_t domain;
};

// Yes, these are a few typedefs which might seem tedious at the first glance, but you'll usually need to do this once per program.
using Core_t = Core<MyCustomRecord>;
using OStreamSink_t = OStreamSink<MyCustomRecord>;
using FileSink_t = FileSink<MyCustomRecord>;
using Logger_t = BaseLogger<MyCustomRecord>;

// Now that our Record class contains an additional domain property, we would like to have a manipulator for our RecordBuilder
using RecordBuilder_t = RecordBuilder<MyCustomRecord>;

class SetDomain
{
public:
	using Domain_t = MyCustomRecord::Domain_t;
	
	explicit SetDomain(Domain_t data) :
		m_Data{ std::move(data) }
	{
	}

	void operator ()(MyCustomRecord& rec)
	{
		rec.domain = std::move(m_Data);
	}

private:
	Domain_t m_Data;
};

inline Core_t gCore;
inline auto gLog = makeLogger<Logger_t>(gCore, SevLvl::info);
inline auto& gConsoleSink
{
	[]() -> auto&
	{
		// let's create the console sink in disabled state. Will become automatically enabled after this scope is left.
		auto wrappedSink = gCore.makeDisabledSink<OStreamSink_t>(std::cout);
		// setting up a custom formatter, thus for each Record only the domain followed by the message will be printed.
		wrappedSink->setFormatter(
								[](const MyCustomRecord& record)
								{
									std::stringstream out;
									out << record.domain << " >> " << record.message();
									return std::move(out).str();
								}
								);

		return *wrappedSink;
	}()
};

int main()
{
	gLog() << "Hello, World!";
	// this clearly belongs to the domain of palindromes, thus let's tag it like this!
	gLog() << SetDomain("palindrome") << "A Man, A Plan, A Canal, Panama!";
}

/* The output of the console looks like this:
 >> Hello, World!
palindrome >> A Man, A Plan, A Canal, Panama!
 
 */
