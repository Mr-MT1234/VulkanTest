#pragma once

#include <vulkan/vulkan.hpp>
#include "abstraction/RenderInstance.h"

class VulkanRenderInstance : public RenderInstance
{
public:
	VulkanRenderInstance(bool debug);
	~VulkanRenderInstance();

	virtual RenderDevice* CreateDevice(const RenderDeviceDesc& desc) const override;

	virtual inline bool IsDebugEnabled() const override { return m_DebugEnabled; };

	virtual inline vk::Instance& getInstance() { return m_Instance; }
private:

	inline vk::SurfaceKHR createSurface(GLFWwindow* window) const;


private:
	vk::Instance m_Instance;
	vk::DispatchLoaderDynamic extFunLoader;

	vk::DebugUtilsMessengerEXT m_DebugMassenger;
	bool m_DebugEnabled;
};