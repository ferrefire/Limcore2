#pragma once

#include <type_traits>
#include <cstddef>
#include <array>
#include <iostream>
#include <cassert>

namespace Limcore
{
	template <typename T>
	concept ValidPointType = (std::is_floating_point<T>().value || std::is_integral<T>().value);

	template <size_t S>
	concept ValidPointRange = (S > 1 && S <= 4);

	#define POINT_TEMPLATE template <ValidPointType T, size_t S> \
		requires ValidPointRange<S>

	#define POINT_INIT_TEMPLATE template <typename... Init> \
		requires (sizeof...(Init) <= S) && (ValidPointType<std::decay_t<Init>> && ...)

	#define POINT_CAST_TEMPLATE template <ValidPointType CT, size_t CS> \
		requires ValidPointRange<CS>

	POINT_TEMPLATE
	class Point
	{
		private:
			std::array<T, S> data{};

		public:
			Point() noexcept = default;
			~Point() noexcept = default;

			POINT_INIT_TEMPLATE
			Point(Init... init) noexcept;

			POINT_CAST_TEMPLATE
			Point(const Point<CT, CS>& other) noexcept;
			POINT_CAST_TEMPLATE
			Point& operator=(const Point<CT, CS>& other) noexcept;

			

			T& operator[](const size_t& i) {assert(i < S); return (data[i]);}
			const T& operator[](const size_t& i) const {assert(i < S); return (data[i]);}
	};

	typedef Point<float, 2> point2D;
	typedef Point<float, 3> point3D;
	typedef Point<float, 4> point4D;

	POINT_TEMPLATE
	std::ostream& operator<<(std::ostream& out, const Point<T, S>& point);
}

#include "point.tpp"