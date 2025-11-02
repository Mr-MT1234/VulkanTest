#pragma once

#include "abstraction/Swapchain.h"
#include <vulkan/vulkan.hpp>
#include <vector>

#include "VulkanImpl/VulkanSurfaceDetails.h"

struct GLFWwindow;

struct VulkanSwapchainDesc : public SwapchainDesc
{
	VulkanSurfaceDetails surfaceDetails;
	std::set<uint32_t> queueFamilies;
	vk::Queue presentationQueue;
};

class VulkanSwapchain : public Swapchain
{
public:
	VulkanSwapchain(vk::Device device, const VulkanSwapchainDesc& desc);
	~VulkanSwapchain();

	virtual SwapchainImage GetNextImage() override;
	virtual void Present(ArrayProxy<Receipe*> receipes) override;
	virtual void ReSize(Dimensions2Du dimensions) { ASSERT(false, "not implemented yet"); }

	inline virtual const ImageDesc& GetImagesDesc() const override { return m_ImagesDesc; };
	inline virtual uint32_t GetImageCount() const override { return m_Images.size(); }
	inline virtual ImageView* GetImageView(uint32_t i) override { return m_Views[i]; }

private:
	vk::Device m_Device;
	vk::SwapchainKHR m_Swapchain;
	vk::Queue m_PresentationQueue;
	vk::Fence m_Fence;

	std::vector<Image*> m_Images;
	std::vector<ImageView*> m_Views;
	ImageDesc m_ImagesDesc;

	uint32_t m_CurrentIndex;
};