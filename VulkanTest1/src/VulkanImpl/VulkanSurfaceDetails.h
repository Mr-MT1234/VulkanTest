#pragma once


#include <vulkan/vulkan.hpp>

struct VulkanSurfaceDetails
{
	vk::SurfaceKHR surface;
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> avlFormats;
	std::vector<vk::PresentModeKHR> avlPresentModes;
};
