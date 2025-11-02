#pragma once

#include "abstraction/RenderDevice.h"
#include "abstraction/Queue.h"
#include <vulkan/vulkan.hpp>

struct VulkanSurfaceDetails;

struct FamilyInfo
{
	uint32_t familyIdx;
	uint32_t count;
	vk::QueueFlags flags;
	bool presentationCapable;
};

struct PhysicalDeviceInfo
{
	vk::PhysicalDevice physicalDevice;
	std::vector<FamilyInfo> familiesInfos;
	std::vector<uint32_t> graphicsQueues;
	std::vector<uint32_t> computeQueues;

	inline uint32_t& getCount(uint32_t i) { return familiesInfos[i].count; };
	inline bool& getPresentCapability(uint32_t i) { return familiesInfos[i].presentationCapable; };
	inline FamilyInfo* getPtr(uint32_t i) { return &familiesInfos[i]; };
};

class VulkanRenderDevice : public RenderDevice
{
public:
	friend class VulkanRenderInstance;

	VulkanRenderDevice(const std::vector<vk::PhysicalDevice>& physicalDevice,vk::SurfaceKHR surface, GLFWwindow* window, bool useGraphics,bool useCompute);
	~VulkanRenderDevice();						

	virtual Buffer* CreateBuffer(const BufferDesc& desc) const override;
		
	inline virtual Swapchain* GetSwapchain() override { return m_Swapchain; }
	inline virtual const Swapchain* GetSwapchain() const override { return m_Swapchain; }
public:
	inline vk::Device getDevice() { return m_Device; }				
	inline vk::Queue getGraphicsQueue() { return m_GraphicsQueue; }
	inline vk::Queue getComputeQueue() { return m_ComputeQueue; }
	inline vk::Queue getPresentationQueue() { return m_PresentationQueue; }
	inline vk::PhysicalDevice getPhysicalDevice() { return m_PhysicalDevice; }
	inline vk::SurfaceKHR getSurface() { return m_Surface; }

private:																												 
	inline PhysicalDeviceInfo selectDevice(vk::SurfaceKHR surface, const std::vector<vk::PhysicalDevice>& physicalDevice,bool useGraphics,bool useCompute);
	inline VulkanSurfaceDetails getSurfaceDetail();
	
private:
	vk::Device m_Device;
	vk::SurfaceKHR m_Surface;
	vk::PhysicalDevice m_PhysicalDevice;

	std::vector<uint32_t> m_QueueFamilies;
	//std::unique_ptr<Queue> m_GraphicsQueue = nullptr;
	//std::unique_ptr<Queue> m_ComputeQueue = nullptr;
	//std::unique_ptr<Queue> m_PresentationQueue = nullptr;

	Swapchain* m_Swapchain;

	vk::Queue m_GraphicsQueue = nullptr;
	vk::Queue m_ComputeQueue = nullptr;
	vk::Queue m_PresentationQueue = nullptr;
};