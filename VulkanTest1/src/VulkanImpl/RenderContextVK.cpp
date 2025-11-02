#include "pch.h"
#include <GLFW/glfw3.h>
#include "RenderContextVK.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData)
{
	constexpr vk::DebugUtilsMessageSeverityFlagBitsEXT severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

	if (messageSeverity >= (VkDebugUtilsMessageSeverityFlagBitsEXT)severity)
	{
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			LOG_TRACE("[Validation layer] Verbose : %s \n%s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			LOG_INFO("[Validation layer] Info : %s \n%s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			LOG_WARN("[Validation layer] Warning : %s \n%s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			ASSERT(false, "[Validation layer] ERROR : %s \n%s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
			break;
		default:
			break;
		}
	}

	return VK_FALSE;
}
inline static vk::DebugUtilsMessengerCreateInfoEXT GetMessagerInfo()
{
	using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

	return vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageSeverity(Severity::eVerbose | Severity::eInfo | Severity::eWarning | Severity::eError)
		.setMessageType(Type::eGeneral | Type::ePerformance | Type::eValidation)
		.setPfnUserCallback(DebugCallback);
}
inline static std::array<const char*,1> GetRequiredDeviceExtensions()
{
	return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}
inline static vk::PhysicalDeviceFeatures GetRequiredDeviceFeatures()
{
	return vk::PhysicalDeviceFeatures();
}

struct QueueFamiliesIndeces
{
	std::optional<uint32_t> graphics;
	std::optional<uint32_t> presentation;

	inline std::array<uint32_t,2> toArray() const
	{
		return std::array<uint32_t,2>{ graphics.value(),presentation.value() };
	}
	inline operator bool() const
	{
		return graphics.has_value() && presentation.has_value();
	}
};
inline static  QueueFamiliesIndeces findQueueFaliliesIndeces(const vk::PhysicalDevice& device, vk::SurfaceKHR surface)
{
	std::vector<vk::QueueFamilyProperties> families = device.getQueueFamilyProperties();

	QueueFamiliesIndeces indeces;
	uint32_t index = 0;
	for (auto& family : families)
	{
		if (family.queueFlags & vk::QueueFlagBits::eGraphics)
			indeces.graphics = index;
		if (device.getSurfaceSupportKHR(index, surface))
			indeces.presentation = index;

		if (indeces)
			break;
		index++;
	}

	return indeces;
}

inline static vk::PhysicalDeviceFeatures operator&(const vk::PhysicalDeviceFeatures& a, const vk::PhysicalDeviceFeatures& b)
{
	constexpr size_t count = sizeof(vk::PhysicalDeviceFeatures) / sizeof(vk::Bool32);

	vk::Bool32* pa = (vk::Bool32*) &a;
	vk::Bool32* pb = (vk::Bool32*) &b;

	vk::PhysicalDeviceFeatures r;
	vk::Bool32* pr = (vk::Bool32*) &r;

	for (size_t i = 0; i < count; i++)
		pr[i] = pa[i] & pb[i];
		
	return r;
}

RenderContextVK::RenderContextVK(GLFWwindow* window,bool enableDebug)
	:debugEnabled(enableDebug)
{
	createInstace();
	createSurface(window);

	if(debugEnabled)
		createDebugMessenger();

	selectePhysicalDevice();
	createLogicalDevice();
}

RenderContextVK::~RenderContextVK()
{
}

inline void RenderContextVK::createInstace()
{
	std::vector<const char*> instanceExtentions;
	std::vector<const char*> instanceLayers;

	if (debugEnabled)
	{
		instanceExtentions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
	}

	auto appInfo = vk::ApplicationInfo().setPApplicationName("Application").setApplicationVersion(VK_MAKE_VERSION(1,0,0))
										.setPEngineName("The Engine").setEngineVersion(VK_MAKE_VERSION(1,0,0))
										.setApiVersion(VK_API_VERSION_1_1);

	auto instanceInfo = vk::InstanceCreateInfo()
		.setEnabledExtensionCount(instanceExtentions.size()).setPpEnabledExtensionNames(instanceExtentions.data())
		.setEnabledLayerCount(instanceLayers.size()).setPpEnabledLayerNames(instanceLayers.data())
		.setPApplicationInfo(&appInfo);

	vk::DebugUtilsMessengerCreateInfoEXT debugInfo = GetMessagerInfo();

	if (debugEnabled)
	{
		instanceInfo.setPNext(&debugInfo);
	}

	instance = vk::createInstance(instanceInfo);

	extFunLoader = vk::DispatchLoaderDynamic(instance, vkGetInstanceProcAddr);
}
inline void RenderContextVK::createDebugMessenger()
{
	vk::DebugUtilsMessengerCreateInfoEXT debugInfo = GetMessagerInfo();
	debugMessager = instance.createDebugUtilsMessengerEXT(debugInfo, nullptr, extFunLoader);
}

inline void RenderContextVK::selectePhysicalDevice()
{
	std::vector<vk::PhysicalDevice> physicalDevices = instance.enumeratePhysicalDevices();
	vk::PhysicalDeviceFeatures requiredFeatures = GetRequiredDeviceFeatures();
	auto requiredExt = GetRequiredDeviceExtensions();


	auto rateDevice = [&](vk::PhysicalDevice& device) -> float
	{
		float score = 0;

		vk::PhysicalDeviceProperties props = device.getProperties();
		vk::PhysicalDeviceFeatures features = device.getFeatures();
		std::vector<vk::ExtensionProperties> availableExt = device.enumerateDeviceExtensionProperties();

		if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
			score += 1000;
		else
			score += 10;

		for (auto& eExt : requiredExt)
		{
			bool found = false;
			for (auto& aExt : availableExt)
			{
				if (std::string(eExt) == aExt.extensionName)
				{
					found = true;
					break;
				}
			}
			if (!found)
				return -10000;
		}

		if ((features & requiredFeatures) != requiredFeatures)
			return -10000;
		if (!findQueueFaliliesIndeces(device, surface))
			return -10000;

		return score;
	};

	float bestScore = -std::numeric_limits<float>::infinity();
	uint32_t bestIndex = -1;
	for (uint32_t i = 0; i < physicalDevices.size(); i++)
	{
		float score = rateDevice(physicalDevices[i]);
		if (score > bestScore)
		{
			bestScore = score;
			bestIndex = i;
		}
	}

	ASSERT(bestScore >= 0, "No Device is Suitable");
	physicalDevice = physicalDevices[bestIndex];
}

inline void RenderContextVK::createSurface(GLFWwindow* window)
{
	VkSurfaceKHR s;
	ASSERT(glfwCreateWindowSurface(instance, window, nullptr, &s) == VK_SUCCESS,"Failed to create a surface");
	surface = s;
}

inline void RenderContextVK::createLogicalDevice()
{
	auto extensions = GetRequiredDeviceExtensions();
	auto features = GetRequiredDeviceFeatures();

	QueueFamiliesIndeces families = findQueueFaliliesIndeces(physicalDevice, surface);
	auto indeces = families.toArray();
	auto last = std::unique(indeces.begin(), indeces.end());

	std::vector<vk::DeviceQueueCreateInfo> queueInfos;
	float priority = 1;
	for (auto it = indeces.begin(); it < last; it++)
	{
		queueInfos.push_back({ {},*it,1,&priority });
	}

	auto deviceInfo = vk::DeviceCreateInfo()
		.setEnabledExtensionCount(extensions.size()).setPpEnabledExtensionNames(extensions.data())
		.setPEnabledFeatures(&features)
		.setQueueCreateInfoCount(queueInfos.size()).setPQueueCreateInfos(queueInfos.data());

	device = physicalDevice.createDevice(deviceInfo);

	queues.graphics = device.getQueue(families.graphics.value(), 0);
	queues.presentation = device.getQueue(families.presentation.value(), 0);
}
