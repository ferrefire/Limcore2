#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <expected>

namespace Limcore
{
	#define RETURN_ERROR(a, b) if (!a) {return (std::unexpected(Error(a.error(), b)));}
	#define RETURN_VK_ERROR(a, b) if (a != VK_SUCCESS) {return (std::unexpected(Error{ErrorCode::VulkanError, b, a}));}

	enum class ErrorCode {Unknown, VulkanError, GlfwError, SwapchainError};

	struct Error
	{
		ErrorCode code;
		std::string message;
		VkResult vulkanResult = VK_SUCCESS;

		Error(ErrorCode code, std::string message) noexcept;
		Error(ErrorCode code, std::string message, VkResult result) noexcept;
		Error(const Error& other, std::string message) noexcept;

		void Print() noexcept;
	};

	template<typename T> using ResultT = std::expected<T, Error>;
	typedef ResultT<void> Result;

	ErrorCode GetErrorType(VkResult result);
	inline bool IsSwapchainError(VkResult result) {return (GetErrorType(result) == ErrorCode::SwapchainError);}
}