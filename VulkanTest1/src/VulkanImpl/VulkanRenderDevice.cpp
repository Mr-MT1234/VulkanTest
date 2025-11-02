#include "pch.h"
#include "VulkanRenderDevice.h"
#include <GLFW/glfw3.h>
#include "VulkanImpl/VulkanQueue.h"
#include "VulkanImpl/VulkanSwapchain.h"
#include "VulkanImpl/VulkanSwapchain.h"
#include "VulkanImpl/VulkanSurfaceDetails.h"
#include "VulkanImpl/VulkanBuffer.h"

constexpr inline static std::array<const char*, 1> GetExtensions()
{
	return {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
}
constexpr inline static vk::PhysicalDeviceFeatures GetFeatures()
{
	vk::PhysicalDeviceFeatures features;
	features.samplerAnisotropy = true;
	return features;
}

inline static vk::PhysicalDeviceFeatures operator&(const vk::PhysicalDeviceFeatures& a, const vk::PhysicalDeviceFeatures& b)
{
	constexpr uint32_t length = sizeof(vk::PhysicalDeviceFeatures) / sizeof(vk::Bool32);

	vk::PhysicalDeviceFeatures result;

	vk::Bool32* aPtr = (vk::Bool32*)&a;
	vk::Bool32* bPtr = (vk::Bool32*)&b;
	vk::Bool32* rPtr = (vk::Bool32*)&result;

	for(uint32_t i = 0; i < length; i++)
	{
		*rPtr = ((*aPtr) && (*bPtr));
		rPtr++;
		aPtr++;
		bPtr++;
	}

	return result;
}

inline static std::vector<uint32_t> SortBySweatability(std::vector<FamilyInfo>& families, vk::QueueFlags req)
{
	auto queueSweatability = [](vk::QueueFlags a, vk::QueueFlags req)
	{
		auto countBits = [](uint32_t c)
		{
			c = c - ((c >> 1) & 0x55555555);
			c = (c & 0x33333333) + ((c >> 2) & 0x33333333);
			return ((c + (c >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
		};

		uint32_t c = (uint32_t)(a & req);
		c *= (c == (uint32_t)req);

		return countBits(c) / std::sqrtf(countBits((uint32_t)a) * countBits((uint32_t)req));
	};

	std::vector<uint32_t> ptrs(families.size());

	for (uint32_t i = 0; i < families.size(); i++)	ptrs[i] = i;

	std::sort(ptrs.begin(), ptrs.end(), [&](uint32_t a, uint32_t b)
		{
			return queueSweatability(families[a].flags, req) > queueSweatability(families[b].flags, req);
		});

	uint32_t i = 0;
	while (queueSweatability(families[ptrs[i]].flags, req) != 0)
	{
		i++;
	}
	ptrs.resize(i);

	return ptrs;
}


VulkanRenderDevice::VulkanRenderDevice(const std::vector<vk::PhysicalDevice>& physicalDevice, vk::SurfaceKHR surface, GLFWwindow* window, bool useGraphics, bool useCompute)
{
	ASSERT(useGraphics | useCompute, "A device with no queues is not allowed");

	constexpr auto extensions = GetExtensions();
	constexpr auto features = GetFeatures();
	constexpr std::array<float, 3> priorities = { 1,1,1 };

	PhysicalDeviceInfo selected = selectDevice(surface, physicalDevice, useGraphics, useCompute);

	std::vector<vk::DeviceQueueCreateInfo> queueInfos; queueInfos.reserve(3);

	FamilyInfo* graphicsFamily = nullptr;
	FamilyInfo* computeFamily = nullptr;
	FamilyInfo* presentationFamily = nullptr;

	//Graphics
	if (useGraphics)
	{
		const uint32_t familyIdx = selected.graphicsQueues[0];
		queueInfos.push_back({ {}, familyIdx,1,priorities.data() });
		selected.getCount(familyIdx)--;
		graphicsFamily = selected.getPtr(familyIdx);
	}
	//Compute
	if (useCompute)
	{
		if ((graphicsFamily == nullptr) | (selected.computeQueues[0] != graphicsFamily->familyIdx))
		{
			const uint32_t familyIdx = selected.computeQueues[0];
			queueInfos.push_back({ {}, familyIdx,1,priorities.data() });
			selected.getCount(familyIdx)--;
			computeFamily = selected.getPtr(familyIdx);
		}
		else
		{
			if (graphicsFamily->count > 0)
			{
				queueInfos[0].queueCount++;
				graphicsFamily->count--;
				computeFamily = graphicsFamily;
			}
			else if(selected.computeQueues.size() > 1)
			{
				const uint32_t familyIdx = selected.computeQueues[1];
				queueInfos.push_back({ {}, familyIdx,1,priorities.data() });
				selected.getCount(familyIdx)--;
				computeFamily = selected.getPtr(familyIdx);
			}
		}

		ASSERT(computeFamily, "Not compatible with app");
	}
	//Presentation
	if (surface)
	{
		if (graphicsFamily != nullptr && graphicsFamily->presentationCapable) presentationFamily = graphicsFamily;
		else if (computeFamily != nullptr && computeFamily->presentationCapable) presentationFamily = computeFamily;
		else
		{
			for (auto& family : selected.familiesInfos)
			{
				if (family.presentationCapable)
				{
					queueInfos.push_back({ {}, family.familyIdx,1,priorities.data() });
					family.count--;
					presentationFamily = &family;
					break;
				}
			}
			ASSERT(presentationFamily, "Not compatible with app");
		}
	}

	auto deviceInfo = vk::DeviceCreateInfo()
		.setPpEnabledExtensionNames(extensions.data()).setEnabledExtensionCount(extensions.size())
		.setPEnabledFeatures(&features)
		.setPQueueCreateInfos(queueInfos.data()).setQueueCreateInfoCount(queueInfos.size());

	m_Device = selected.physicalDevice.createDevice(deviceInfo);
	m_Surface = surface;
	//if (graphicsFamily)
	//	m_GraphicsQueue = std::make_unique<VulkanQueue>(m_Device.getQueue(graphicsFamily->familyIdx, 0));
	//if (computeFamily)
	//	m_GraphicsQueue = std::make_unique<VulkanQueue>(m_Device.getQueue(computeFamily->familyIdx, (computeFamily == graphicsFamily)));
	//if (presentationFamily)
	//	m_GraphicsQueue = std::make_unique<VulkanQueue>(m_Device.getQueue(presentationFamily->familyIdx, 0));
	std::vector<uint32_t> selectedFamilies; selectedFamilies.reserve(3);
	if (graphicsFamily)
	{
		m_GraphicsQueue = m_Device.getQueue(graphicsFamily->familyIdx, 0);
		selectedFamilies.push_back(graphicsFamily->familyIdx);
	}
	if (computeFamily)
	{
		m_ComputeQueue = m_Device.getQueue(computeFamily->familyIdx, (computeFamily == graphicsFamily));
		selectedFamilies.push_back(computeFamily->familyIdx);
	}
	if (presentationFamily)
	{ 
		m_PresentationQueue = m_Device.getQueue(presentationFamily->familyIdx, 0);
		selectedFamilies.push_back(presentationFamily->familyIdx);
	}

	m_PhysicalDevice = selected.physicalDevice;
	for (const auto& q : queueInfos)
		m_QueueFamilies.push_back(q.queueFamilyIndex);

	//Swapchain
	VulkanSwapchainDesc swapchainDesc;
	{
		int w, h;	glfwGetWindowSize(window, &w, &h);
		auto swapDetails = getSurfaceDetail();

		swapchainDesc.presentMode = PresentMode::VSync;
		swapchainDesc.imagesDimensions = { (uint32_t)w,(uint32_t)h };
		swapchainDesc.presentationQueue = m_PresentationQueue;
		swapchainDesc.queueFamilies = { selectedFamilies.begin(),selectedFamilies.end() };
		swapchainDesc.surfaceDetails = getSurfaceDetail();
	}

	m_Swapchain = new VulkanSwapchain(m_Device, swapchainDesc);
}

VulkanRenderDevice::~VulkanRenderDevice()
{
	delete m_Swapchain;
	m_Device.destroy();
}

Buffer* VulkanRenderDevice::CreateBuffer(const BufferDesc& desc) const
{
	return new VulkanBuffer(m_Device, { desc,m_PhysicalDevice,m_QueueFamilies });
}

/*TODO: Rewrite
	- Queue selection could be better.
	- Performance (not very important)
*/
inline PhysicalDeviceInfo VulkanRenderDevice::selectDevice(vk::SurfaceKHR surface, const std::vector<vk::PhysicalDevice>& physicalDevices, bool useGraphics, bool useCompute)
{
	auto reqExtensions = GetExtensions();
	auto reqFeatures = GetFeatures();

	float bestScore = -std::numeric_limits<float>::infinity();
	uint32_t bestDeviceIdx = -1;

	uint32_t i = 0;
	std::vector<FamilyInfo> bestFamilies;
	std::vector<uint32_t> bestGraphics;
	std::vector<uint32_t> bestCompute;

	for (const auto& device : physicalDevices)
	{
		float score = 0;

		auto avlExtensions = device.enumerateDeviceExtensionProperties();
		auto avlFamilies = device.getQueueFamilyProperties();
		auto avlFeatures = device.getFeatures();
		auto props = device.getProperties();

		std::vector<FamilyInfo> familiesInfos(avlFamilies.size());
		std::vector<uint32_t> sortedGraphics;
		std::vector<uint32_t> sortedCompute;

		for (uint32_t i = 0; i < avlFamilies.size(); i++) familiesInfos[i] = { i,avlFamilies[i].queueCount,avlFamilies[i].queueFlags,false };

		//Check extension support
		{
			for (const auto& req : reqExtensions)
			{
				bool found = false;
				for (const auto& avl : avlExtensions)
					if (strcmp(req, avl.extensionName) == 0)
					{
						found = true;
						break;
					}

				if (!found)
				{
					score = -10000;
					goto END;
				}
			}
		}
		//Check feature support
		{
			if ((avlFeatures & reqFeatures) != reqFeatures)
			{
				score = -10000;
				goto END;
			}
		}
		//Check family support
		{
			//Graphics
			if (useGraphics)
			{
				constexpr vk::QueueFlags req = vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;

				sortedGraphics = SortBySweatability(familiesInfos, req);

				if (sortedGraphics.size() == 0)
				{
					score = -10000;
					goto END;
				}
			}
			//Compute
			if (useCompute)
			{
				constexpr vk::QueueFlags req = vk::QueueFlagBits::eCompute | vk::QueueFlagBits::eTransfer;

				sortedCompute = SortBySweatability(familiesInfos, req);

				if (sortedCompute.size() == 0)
				{
					score = -10000;
					goto END;
				}
			}
			//Presenting
			if (surface)
			{
				bool doesSupport = false;
				for (auto& f : familiesInfos)
				{
					doesSupport |= (f.presentationCapable = device.getSurfaceSupportKHR(f.familyIdx,surface));
				}
				if (!doesSupport)
				{
					score = -10000;
					goto END;
				}
			}

		}

		if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;
		if (surface && useGraphics && sortedGraphics[0]) score += 1000;
		if (useGraphics && useCompute && (sortedGraphics[0] != sortedCompute[0])) score += 100;

	END:
		if (score > bestScore)
		{
			bestDeviceIdx = i;
			bestScore = score;

			bestFamilies = std::move(familiesInfos);
			bestGraphics = std::move(sortedGraphics);
			bestCompute = std::move(sortedCompute);
		}

		i++;
	}


	ASSERT(bestScore >= 0, "Failed to find a sweatable physicale device");

	return 
	{
		physicalDevices[bestDeviceIdx],
		bestFamilies,
		bestGraphics,
		bestCompute
	};

	/*			auto queueSweatability = [](vk::QueueFlags a, vk::QueueFlags req)
			{
				auto countBits = [](uint32_t c)
				{
					c = c - ((c >> 1) & 0x55555555);
					c = (c & 0x33333333) + ((c >> 2) & 0x33333333);
					return ((c + (c >> 4) & 0xF0F0F0F) * 0x1010101) >> 24;
				};

				uint32_t c = (uint32_t)(a & req);
				c *= (c == (uint32_t)req);

				return countBits(c) / std::sqrtf(countBits((uint32_t)a) * countBits((uint32_t)req));
			};
	*/
}

inline VulkanSurfaceDetails VulkanRenderDevice::getSurfaceDetail()
{
	return {
		m_Surface,
		m_PhysicalDevice.getSurfaceCapabilitiesKHR(m_Surface),
		m_PhysicalDevice.getSurfaceFormatsKHR(m_Surface),
		m_PhysicalDevice.getSurfacePresentModesKHR(m_Surface),
	};
}
