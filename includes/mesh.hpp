#pragma once

#include "point.hpp"
#include "utility.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <vector>

namespace Limcore
{
	typedef enum VertexConfigBits
	{
		None = 0,
		Position = 1 << 0,
		Normal = 1 << 1,
		Coordinate = 1 << 2,
		Color = 1 << 3
	} VertexConfigBits;
	typedef uint32_t VertexConfig;

	struct PositionStruct {point3D position;};
	struct NormalStruct {point3D normal;};
	struct CoordinateStruct {point2D coordinate;};
	struct ColorStruct {point4D color;};

	template <VertexConfig V>
	struct Empty {};

	template <VertexConfig V>
	struct Vertex : 
		std::conditional_t<HasFlag(V, Position), PositionStruct, Empty<Position>>,
		std::conditional_t<HasFlag(V, Normal), NormalStruct, Empty<Normal>>,
		std::conditional_t<HasFlag(V, Coordinate), CoordinateStruct, Empty<Coordinate>>,
		std::conditional_t<HasFlag(V, Color), ColorStruct, Empty<Color>>
	{
		explicit operator std::vector<float>() const noexcept;
	};

	#define MESH_TEMPLATE template <VertexConfig V, VkIndexType I> \
		requires (I == VK_INDEX_TYPE_UINT16 || I == VK_INDEX_TYPE_UINT32 || I == VK_INDEX_TYPE_NONE_KHR)

	MESH_TEMPLATE
	class Mesh
	{
		using indexType = typename std::conditional_t<I == VK_INDEX_TYPE_UINT32, uint32_t, uint16_t>;
		//constexpr bool hasIndices = I != VK_INDEX_TYPE_NONE_KHR;

		private:
			

		public:
			Mesh() noexcept = default;
			~Mesh() noexcept {Destroy();}

			Mesh(const Mesh&) = delete;
			Mesh& operator=(const Mesh&) = delete;

			//Todo: Add move operators.

			void Destroy() noexcept;
	};
}