#pragma once

#include "Dimensions.h"
#include "abstraction/CommonEnums.h"

struct ImageDesc
{
	Dimensions3Du dimensions;
	uint32_t layers;
	ImageFormat format;
	ImageType type;

	ResourceAccessRate gpuAccessRate = ResourceAccessRate::Frequent;
	ResourceAccessibilityBits cpuAccessibility = ResourceAccessibilityBits::None;
};

class Image
{
public:
	virtual ~Image() = default;

	virtual const ImageDesc& GetDesc() = 0;
};