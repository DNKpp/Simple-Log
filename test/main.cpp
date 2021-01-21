//          Copyright Dominic Koepke 2020 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include <iostream>

int main(int argc, char** argv)
{
	const int result = Catch::Session().run(argc, argv);
	return result;
}
