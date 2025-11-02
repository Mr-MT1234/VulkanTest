#pragma once

#include <stdint.h>
#include "abstraction/ImageView.h"
#include "abstraction/Receipe.h"
#include "Dimensions.h"
#include "ArrayProxy.h"

struct SwapchainImage
{
	ImageView* image;
	uint32_t index;

	inline constexpr operator ImageView* () const
	{
		return image;
	}
};

enum class PresentMode
{
	Imidiate, VSync
};

struct SwapchainDesc
{
	Dimensions2Du imagesDimensions;
	PresentMode presentMode;
};

class Swapchain
{
public:
	virtual ~Swapchain() = default;

	virtual SwapchainImage GetNextImage() = 0;
	virtual void Present(ArrayProxy<Receipe*> receipes) = 0;
	virtual void ReSize(Dimensions2Du dimensions) = 0;

	virtual ImageView* GetImageView(uint32_t i) = 0;
	virtual const ImageDesc& GetImagesDesc() const = 0;
	virtual uint32_t GetImageCount() const = 0;
};