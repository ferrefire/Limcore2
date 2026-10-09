#include "point.hpp"

#include <cmath>

namespace Limcore
{
	POINT_TEMPLATE
	POINT_INIT_TEMPLATE
	Point<T, S>::Point(Init... init) noexcept
	{
		size_t i = 0;
		((data[i++] = static_cast<T>(init)), ...);
		if (sizeof...(Init) == 1) {for (i = 1; i < S; i++) {data[i] = data[0];}}
	}

	POINT_TEMPLATE
	POINT_CAST_TEMPLATE
	Point<T, S>::Point(const Point<CT, CS>& other) noexcept
	{
		size_t s = (S < CS ? S : CS);
		for (size_t i = 0; i < s; i++) {data[i] = static_cast<T>(other[i]);}
	}

	POINT_TEMPLATE
	POINT_CAST_TEMPLATE
	Point<T, S>& Point<T, S>::operator=(const Point<CT, CS>& other) noexcept
	{
		size_t s = (S < CS ? S : CS);
		for (size_t i = 0; i < s; i++) {data[i] = static_cast<T>(other[i]);}

		return (*this);
	}

	POINT_TEMPLATE
	POINT_INIT_CAST_TEMPLATE
	Point<T, S>::Point(const Point<CT, CS>& other, Init... init) noexcept
	{
		size_t s = (S < CS ? S : CS);
		size_t i = 0;
		for (i = 0; i < s; i++) {data[i] = static_cast<T>(other[i]);}

		((data[i++] = static_cast<T>(init)), ...);
	}

	POINT_TEMPLATE
	constexpr double Point<T, S>::LengthSquared() const noexcept
	{
		double totalSquared = 0;
		for (size_t i = 0; i < S; i++) {totalSquared += (data[i] * data[i]);}

		return (totalSquared);
	}

	POINT_TEMPLATE
	constexpr double Point<T, S>::Length() const noexcept
	{
		double totalSquared = LengthSquared();
		double total = sqrt(totalSquared);

		return (total);
	}

	POINT_TEMPLATE
	constexpr void Point<T, S>::Rotate(const double& degrees) noexcept requires(S == 2)
	{
		const double radians = degrees * DEGREES_TO_RADIANS;
		const double cosTheta = cos(radians);
		const double sinTheta = sin(radians);
		const Point temp = *this;

		x() = (temp.x() * cosTheta) - (temp.y() * sinTheta);
		y() = (temp.x() * sinTheta) + (temp.y() * cosTheta);
	}

	POINT_TEMPLATE
	constexpr void Point<T, S>::Rotate(const double& degrees, const Axis& axis) noexcept requires(S == 3)
	{
		const double radians = degrees * DEGREES_TO_RADIANS;
		const double cosTheta = cos(radians);
		const double sinTheta = sin(radians);
		
		const size_t ai = (axis != Axis::X ? 0 : 1);
		const size_t bi = (axis != Axis::Z ? 2 : 1);

		const double a = static_cast<double>(data[ai]);
		const double b = static_cast<double>(data[bi]);

		data[ai] = (a * cosTheta) - (b * sinTheta);
		data[bi] = (a * sinTheta) + (b * cosTheta);
	}

	POINT_TEMPLATE
	constexpr void Point<T, S>::Rotate(const Point<double, 3>& rotation) noexcept requires(S == 3)
	{
		if (rotation.x() != 0) {Rotate(rotation.x(), Axis::X);}
		if (rotation.y() != 0) {Rotate(rotation.y(), Axis::Y);}
		if (rotation.z() != 0) {Rotate(rotation.z(), Axis::Z);}
	}

	POINT_TEMPLATE
	std::ostream& operator<<(std::ostream& out, const Point<T, S>& point)
	{
		for (size_t i = 0; i < S; i++) {out << point[i] << (i + 1 < S ? ", " : "");}

		return (out);
	}
}