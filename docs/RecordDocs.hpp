//          Copyright Dominic Koepke 2021 - 2021.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** \addtogroup Record
 * @{
 * \details Records are basically a collection of information, which has been gathered either implicitly or explicitly while a logging Message was created. It is not intended that users ever use a plain Record or set it up by theirselves.
 * The library provides a simple interface which does all the trivial part of a logging step, while users can concentrate on setting up their message.\n Have a look at Logger and RecordBuilder if you want to know more about how to setup Records.
 *
 * \section Custom_Record_Types Custom Record Types
 * In the \ref PreparedTypes "preset namespace" of this library users can find a Record_t alias, which will accept values of the \ref preset::SevLvl "SevLvl" enum as SeverityLevel and std::strings as Channel type. Users may exchange any or all of these
 * predefined types with their own. If you use any Filter, FlushPolicy or anything else which watches out for a specific Channel, it might be wise to exchange the std::string type with something less expensive to compare, e.g. an enum type or plain int.
 * For example users could use the BaseRecord type, but exchanging some member types with any other type or even create their custom Record type from scratch. Here is an example for the former:
 *
 * \dontinclude{lineno} CustomizeBaseRecord\main.cpp
 * \skip Simple-Log/
 * \until network >>
 *
 * If your needs are more specific and you want to add things such as custom properties to the Record type, this might be an example for you:
 * \dontinclude{lineno} CustomRecordType\main.cpp
 * \skip Simple-Log/
 * \until palindrome >>
 */

/** @}*/
