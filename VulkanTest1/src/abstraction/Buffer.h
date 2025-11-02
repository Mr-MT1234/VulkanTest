#pragma once

#include "CommonEnums.h"

struct BufferDesc
{
	BufferUsageFlags usage = BufferUsageBits::NoUse;
	size_t size = 0;
	ResourceAccessRate gpuAccessRate = ResourceAccessRate::Frequent;
	ResourceAccessibilityBits cpuAccessibility = ResourceAccessibilityBits::None;
};

class Buffer
{
public:
	virtual ~Buffer() = default;

	virtual void* Map() = 0;
	virtual void UnMap() = 0;
	virtual const BufferDesc& GetDesc() const = 0;
};