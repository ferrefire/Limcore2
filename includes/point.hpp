#pragma once

#include <type_traits>
#include <cstddef>
#include <array>
#include <iostream>
#include <cassert>

namespace Limcore
{
	#define DEGREES_TO_RADIANS 0.0174532925

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
	
	#define POINT_INIT_CAST_TEMPLATE template<ValidPointType CT, size_t CS, typename... Init> \
		requires (ValidPointRange<CS>) && (sizeof...(Init) <= S - CS) && (ValidPointType<std::decay_t<Init>> && ...)

	enum class Axis {X, Y, Z};

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

			POINT_INIT_CAST_TEMPLATE
			Point(const Point<CT, CS>& other, Init... init) noexcept;

			constexpr T& x() noexcept {return (data[0]);}
			constexpr T& y() noexcept requires(S > 1) {return (data[1]);}
			constexpr T& z() noexcept requires(S > 2) {return (data[2]);}
			constexpr T& w() noexcept requires(S > 3) {return (data[3]);}

			constexpr const T& x() const noexcept {return (data[0]);}
			constexpr const T& y() const noexcept requires(S > 1) {return (data[1]);}
			constexpr const T& z() const noexcept requires(S > 2) {return (data[2]);}
			constexpr const T& w() const noexcept requires(S > 3) {return (data[3]);}

			constexpr T& operator[](const size_t& i) {assert(i < S); return (data[i]);}
			constexpr const T& operator[](const size_t& i) const {assert(i < S); return (data[i]);}

			constexpr void operator+=(const Point& other) noexcept {for (size_t i = 0; i < S; i++) {data[i] += other[i];}}
			constexpr void operator-=(const Point& other) noexcept {for (size_t i = 0; i < S; i++) {data[i] -= other[i];}}
			constexpr void operator*=(const Point& other) noexcept {for (size_t i = 0; i < S; i++) {data[i] *= other[i];}}
			constexpr void operator/=(const Point& other) noexcept {for (size_t i = 0; i < S; i++) {data[i] /= other[i];}}

			constexpr Point operator+(const Point& other) const noexcept {Point result = *this; result += other; return (result);}
			constexpr Point operator-(const Point& other) const noexcept {Point result = *this; result -= other; return (result);}
			constexpr Point operator*(const Point& other) const noexcept {Point result = *this; result *= other; return (result);}
			constexpr Point operator/(const Point& other) const noexcept {Point result = *this; result /= other; return (result);}

			constexpr double LengthSquared() const noexcept;
			constexpr double Length() const noexcept;
			constexpr double MagnitudeSquared() const noexcept {return (LengthSquared());}
			constexpr double Magnitude() const noexcept {return (Length());}

			constexpr void Normalize() noexcept {*this /= Length();}
			constexpr Point Normalized() const noexcept {return (*this / Length());}

			constexpr void Rotate(const double& degrees) noexcept requires(S == 2);
			constexpr void Rotate(const double& degrees, const Axis& axis) noexcept requires(S == 3);
			constexpr void Rotate(const Point<double, 3>& rotation) noexcept requires(S == 3);
	};

	typedef Point<float, 2> point2D;
	typedef Point<float, 3> point3D;
	typedef Point<float, 4> point4D;
	typedef Point<double, 2> dpoint2D;
	typedef Point<double, 3> dpoint3D;
	typedef Point<double, 4> dpoint4D;
	typedef Point<int, 2> ipoint2D;
	typedef Point<int, 3> ipoint3D;
	typedef Point<int, 4> ipoint4D;

	POINT_TEMPLATE
	std::ostream& operator<<(std::ostream& out, const Point<T, S>& point);
}

#include "point.tpp"