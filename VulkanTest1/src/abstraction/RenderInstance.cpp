#include "pch.h"
#include "RenderInstance.h"
#include "VulkanImpl/VulkanRenderInstance.h"

RenderInstance* RenderInstance::Create(bool debugEnabled)
{
	return new VulkanRenderInstance(debugEnabled);
}
