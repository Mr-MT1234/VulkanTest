#include "pch.h"
#include "VulkanRenderInstance.h"
#include "VulkanRenderDevice.h"

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	return VK_FALSE;
}

static inline vk::DebugUtilsMessengerCreateInfoEXT GetMessengerInfo()
{
	using Severety = vk::DebugUtilsMessageSeverityFlagBitsEXT;
	using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

	return vk::DebugUtilsMessengerCreateInfoEXT()
		.setMessageSeverity(Severety::eVerbose | Severety::eInfo | Severety::eWarning | Severety::eError)
		.setMessageType(Type::eGeneral | Type::ePerformance | Type::eValidation)
		.setPfnUserCallback(DebugCallback);
}

VulkanRenderInstance::VulkanRenderInstance(bool debug)
	:m_DebugEnabled(debug)
{
	vk::ApplicationInfo appInfo = { 
									"an Application",VK_MAKE_VERSION(1,0,0),
									"an Engine" ,VK_MAKE_VERSION(1,0,0),
									VK_API_VERSION_1_1
								  };

	uint32_t count;
	const char** glfwExt = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> extensions(glfwExt, glfwExt + count);
	std::vector<const char*> layers;

	if (m_DebugEnabled)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		layers.push_back("VK_LAYER_KHRONOS_validation");
	}

	auto instanceInfo = vk::InstanceCreateInfo()
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(extensions.size()).setPpEnabledExtensionNames(extensions.data())
		.setEnabledLayerCount(layers.size()).setPpEnabledLayerNames(layers.data());

	vk::DebugUtilsMessengerCreateInfoEXT messangerInfo = GetMessengerInfo();

	if (m_DebugEnabled)
		instanceInfo.setPNext(&messangerInfo);

	m_Instance = vk::createInstance(instanceInfo);

	extFunLoader = vk::DispatchLoaderDynamic(m_Instance, vkGetInstanceProcAddr);

	if (m_DebugEnabled)
		m_DebugMassenger = m_Instance.createDebugUtilsMessengerEXT(messangerInfo,nullptr,extFunLoader);
}

VulkanRenderInstance::~VulkanRenderInstance()
{
	if (m_DebugEnabled)
		m_Instance.destroyDebugUtilsMessengerEXT(m_DebugMassenger, nullptr, extFunLoader);
	m_Instance.destroy();
}


RenderDevice* VulkanRenderInstance::CreateDevice(const RenderDeviceDesc& desc) const
{
	return new VulkanRenderDevice(m_Instance.enumeratePhysicalDevices(), desc.window ? createSurface(desc.window) : vk::SurfaceKHR(nullptr),desc.window, desc.useGraphics, desc.useCompute);
}


inline vk::SurfaceKHR VulkanRenderInstance::createSurface(GLFWwindow* window) const
{
	VkSurfaceKHR s;
	ASSERT(glfwCreateWindowSurface(m_Instance, window, nullptr, &s) == VK_SUCCESS, "Failed to create a surface");
	return vk::SurfaceKHR(s);
}


