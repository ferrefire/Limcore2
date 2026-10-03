#include "error.hpp"

#include "printer.hpp"

#include <iostream>

namespace Limcore
{
	Error::Error(ErrorCode code, std::string message) noexcept : code(code), message(message) {}

	Error::Error(ErrorCode code, std::string message, VkResult result) noexcept : code(code), message(message), vulkanResult(result)
	{
		this->message.append(" with VkResult: ");
		std::string_view resultName = EnumName(result);
		this->message.append((resultName == "UNKOWN" ? std::to_string(result) : resultName));
	}

	Error::Error(const Error& other, std::string message) noexcept
	{
		this->vulkanResult = other.vulkanResult;
		code = other.code;
		this->message = message;
		this->message.append(": ");
		this->message.append(other.message);
	}

	void Error::Print() noexcept
	{
		std::cerr << "Error: " << ENUM_VAL(code) << " " << VAR_VAL(message) << std::endl;
	}

	ErrorCode GetErrorType(VkResult result)
	{
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {return (ErrorCode::SwapchainError);}

		return (ErrorCode::Unknown);
	}
}