#pragma once

#include <string>
#include "abstraction/Swapchain.h"
#include "abstraction/Buffer.h"

struct GLFWwindow;

struct RenderDeviceDesc
{
	GLFWwindow* window = nullptr;
	bool useGraphics = true;
	bool useCompute = false;
};

class RenderDevice
{
public:
	virtual ~RenderDevice() = default;

	virtual Swapchain* GetSwapchain() = 0;
	virtual const Swapchain* GetSwapchain() const = 0;

	virtual Buffer* CreateBuffer(const BufferDesc& desc) const = 0;
};