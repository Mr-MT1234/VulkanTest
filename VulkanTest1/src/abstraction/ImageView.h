#pragma once

#include "abstraction/Image.h"
#include "abstraction/CommonEnums.h"

struct ImageViewDesc
{
	Image* image = nullptr;
	Dimensions3Du dimensions;
	uint32_t baseLayer = 0;
	uint32_t layers;
	ImageViewType type;
	ImageViewAspect aspect;
};

class ImageView
{
public:
	virtual ~ImageView() = default;
};