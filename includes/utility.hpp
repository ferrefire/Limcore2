#pragma once

#include <functional>

#define CUI(a) static_cast<uint32_t>(a)

namespace Limcore
{
	template <typename M, typename F>
	static constexpr bool HasFlag(M mask, F flag) {return ((mask & flag) == flag);} //Todo: Remove static.

	template <typename M, typename F>
	static constexpr M SetFlag(M mask, F flag) {return (mask | flag);} //Todo: Remove static.

	template <class T>
	std::function<void()> BindFunction(T* object, void(T::*call)()) {return (std::bind_front(call, object));} //Todo: Make constexpr.
}