#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <expected>

namespace Limcore
{
	enum class ErrorCode {Unknown, VulkanError, GlfwError};

	struct Error
	{
		ErrorCode code;
		std::string message;

		Error(ErrorCode code, std::string message) noexcept;
		Error(ErrorCode code, std::string message, VkResult result) noexcept;
		Error(const Error& other, std::string message) noexcept;

		void Print() noexcept;
	};

	template<typename T> using Result = std::expected<T, Error>;
}