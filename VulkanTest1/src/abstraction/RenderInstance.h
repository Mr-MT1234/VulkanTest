#pragma once

#include "abstraction/RenderDevice.h"
#include <memory>

struct GLFWwindow;

class RenderInstance
{
public:
	static RenderInstance* Create(bool debugEnabled);
public:
	virtual ~RenderInstance() = default;

	virtual RenderDevice* CreateDevice(const RenderDeviceDesc& desc) const = 0;
	
	virtual bool IsDebugEnabled() const = 0;
};