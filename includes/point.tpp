#include "point.hpp"

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
	std::ostream& operator<<(std::ostream& out, const Point<T, S>& point)
	{
		for (size_t i = 0; i < S; i++) {out << point[i] << (i + 1 < S ? ", " : "");}

		return (out);
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
}