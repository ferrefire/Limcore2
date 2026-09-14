#include "application.hpp"

#include <iostream>

int main()
{
	Limcore::Result<VkInstance> result = Limcore::CreateInstance();
	if (!result)
	{
		std::cerr << result.error().message << std::endl;
		return (0);
	}

	VkInstance instance = result.value();

	std::cout << "yahoo!" << std::endl;

	vkDestroyInstance(instance, nullptr);

	glfwTerminate();

	std::cout << "instance destroyed." << std::endl;

	return (0);
}