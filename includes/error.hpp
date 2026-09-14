#pragma once

#include <string>
#include <expected>

namespace Limcore
{
	enum class ErrorCode {Unknown, VulkanError, GlfwError};

	struct Error
	{
		ErrorCode code;
		std::string message;
	};

	template<typename T> using Result = std::expected<T, Error>;
}