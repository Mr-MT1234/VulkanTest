#pragma once

#include "abstraction/CommonEnums.h"
#include <vulkan/vulkan.hpp>
#include "Defines.h"

static constexpr inline vk::Format GetVkFormat(ImageFormat f)
{
	switch (f)
	{
    case ImageFormat::RGBA8:            return vk::Format::eR8G8B8A8Srgb;
    case ImageFormat::RGB8:             return vk::Format::eR8G8B8Srgb;
    case ImageFormat::RG8:              return vk::Format::eR8G8Srgb;
    case ImageFormat::R8:               return vk::Format::eR8Srgb;
    case ImageFormat::RGBA16:           return vk::Format::eR16G16B16A16Sfloat;
    case ImageFormat::RGB16:            return vk::Format::eR16G16B16Sfloat;
    case ImageFormat::RG16:             return vk::Format::eR16G16Sfloat;
    case ImageFormat::R16:              return vk::Format::eR16Sfloat;
    case ImageFormat::RGBA32:           return vk::Format::eR32G32B32A32Sfloat;
    case ImageFormat::RGB32:            return vk::Format::eR32G32B32Sfloat;
    case ImageFormat::RG32:             return vk::Format::eR32G32Sfloat;
    case ImageFormat::R32:              return vk::Format::eR32Sfloat;
    case ImageFormat::Depth32:          return vk::Format::eD32Sfloat;
    case ImageFormat::Depth16:          return vk::Format::eD16Unorm;
    case ImageFormat::Stincel8:         return vk::Format::eS8Uint;
    case ImageFormat::Depth24Stencel8:  return vk::Format::eD24UnormS8Uint;
    default:
        ASSERT(false, "Unknown format");
	}
}
static constexpr inline ImageFormat GetFormat(vk::Format f)
{
    switch (f)
    {
    case vk::Format::eR8G8B8A8Srgb:         return  ImageFormat::RGBA8;
    case vk::Format::eR8G8B8Srgb:           return  ImageFormat::RGB8;
    case vk::Format::eR8G8Srgb:             return  ImageFormat::RG8;
    case vk::Format::eR8Srgb:               return  ImageFormat::R8;
    case vk::Format::eR16G16B16A16Sfloat:   return  ImageFormat::RGBA16;
    case vk::Format::eR16G16B16Sfloat:      return  ImageFormat::RGB16;
    case vk::Format::eR16G16Sfloat:         return  ImageFormat::RG16;
    case vk::Format::eR16Sfloat:            return  ImageFormat::R16;
    case vk::Format::eR32G32B32A32Sfloat:   return  ImageFormat::RGBA32;
    case vk::Format::eR32G32B32Sfloat:      return  ImageFormat::RGB32;
    case vk::Format::eR32G32Sfloat:         return  ImageFormat::RG32;
    case vk::Format::eR32Sfloat:            return  ImageFormat::R32;
    case vk::Format::eD32Sfloat:            return  ImageFormat::Depth32;
    case vk::Format::eD16Unorm:             return  ImageFormat::Depth16;
    case vk::Format::eS8Uint:               return  ImageFormat::Stincel8;
    case vk::Format::eD24UnormS8Uint:       return  ImageFormat::Depth24Stencel8;
    default:
        ASSERT(false, "Unknown vkFormat");
    }
}

static constexpr inline vk::ImageViewType GetVkViewType(ImageViewType t)
{
    switch (t)
    {
    case ImageViewType::e1D:        return vk::ImageViewType::e1D;
    case ImageViewType::e2D:        return vk::ImageViewType::e2D;
    case ImageViewType::e3D:        return vk::ImageViewType::e3D;
    case ImageViewType::e1DArray:   return vk::ImageViewType::e1DArray;
    case ImageViewType::e2DArray:   return vk::ImageViewType::e2DArray;
    case ImageViewType::CubeArray:  return vk::ImageViewType::eCubeArray;
    case ImageViewType::Cube:       return vk::ImageViewType::eCube;
    default:
        ASSERT(false, "Unknown Image view type");
    }
}
static constexpr inline ImageViewType GetVIewType(vk::ImageViewType t)
{
    switch (t)
    {
    case vk::ImageViewType::e1D:        return ImageViewType::e1D;
    case vk::ImageViewType::e2D:        return ImageViewType::e2D;
    case vk::ImageViewType::e3D:        return ImageViewType::e3D;
    case vk::ImageViewType::e1DArray:   return ImageViewType::e1DArray;
    case vk::ImageViewType::e2DArray:   return ImageViewType::e2DArray;
    case vk::ImageViewType::eCubeArray: return ImageViewType::CubeArray;
    case vk::ImageViewType::eCube:      return ImageViewType::Cube;
    default:
        ASSERT(false, "Unknown Image view type");
    }
}

static constexpr inline vk::ImageType GetVkType(ImageType type)
{
    switch (type)
    {
    case ImageType::e1D:    return vk::ImageType::e1D;
    case ImageType::e2D:    return vk::ImageType::e2D;
    case ImageType::e3D:    return vk::ImageType::e3D;
    }
}

static constexpr inline vk::ImageAspectFlags GetVkImageAspect(ImageViewAspect aspect)
{
    switch (aspect)
    {
    case ImageViewAspect::Color:    return vk::ImageAspectFlagBits::eColor;
    case ImageViewAspect::Depth:    return vk::ImageAspectFlagBits::eDepth;
    case ImageViewAspect::Stencil:  return vk::ImageAspectFlagBits::eStencil;
    }
}



















