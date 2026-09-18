#include "error.hpp"

#include "printer.hpp"

#include <iostream>

namespace Limcore
{
	Error::Error(ErrorCode code, std::string message) noexcept : code(code), message(message) {}

	Error::Error(const Error& other, std::string message) noexcept
	{
		code = other.code;
		this->message = message;
		this->message.append(": ");
		this->message.append(other.message);
	}

	void Error::Print() noexcept
	{
		std::cerr << "Error: " << ENUM_VAL(code) << " " << VAR_VAL(message) << std::endl;
	}
}