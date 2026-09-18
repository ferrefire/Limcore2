#pragma once

#include <iostream>
#include <typeinfo>

#if defined(__clang__) || defined(__GNUC__)
#	define PRETTY_FUN __PRETTY_FUNCTION__
#	define PRETTY_KEY "T = "
#	define PRETTY_SLOT "];>"
#elif defined(_MSC_VER)
#	define PRETTY_FUN __FUNCSIG__
#	define PRETTY_KEY "StringName<"
#	define PRETTY_SLOT ">"
#else
#	error Unsupported compiler
#endif

template <auto T>
constexpr std::string_view StringName()
{
	constexpr std::string_view stringLiteral = PRETTY_FUN;	
	constexpr std::string_view key = PRETTY_KEY;
	constexpr std::string_view slot = PRETTY_SLOT;

	const std::size_t start = stringLiteral.find(key) + key.size();
	const std::size_t end = stringLiteral.find_first_of(slot, start);
	
	return (stringLiteral.substr(start, end - start));
}

template <typename T, T... elements>
struct EnumMap
{
	static constexpr std::string_view names[sizeof...(elements)] = {StringName<elements>()...};
};

template <typename T, std::size_t... elements>
constexpr auto CreateMap(int val, std::index_sequence<elements...> sequence)
{
	return (EnumMap<T, static_cast<T>(elements)...>{});
}

template <typename T>
constexpr std::string_view EnumName(T val)
{
	if (static_cast<int>(val) < 0 || static_cast<int>(val) > 255) return ("UNKOWN");
	const auto map = CreateMap<T>(static_cast<int>(val), std::make_index_sequence<256>{});

	return (map.names[static_cast<int>(val)]);
}

#define VAR_VAL(v) #v << ": " << v
#define ENUM_VAL(e) #e << ": " << EnumName(e)