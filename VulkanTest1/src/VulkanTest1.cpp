#include "pch.h"
#include "stb_image.h"
#include "abstraction/RenderInstance.h"
#include "VulkanImpl/VulkanRenderInstance.h"
#include "abstraction/RenderDevice.h"
#include "VulkanImpl/VulkanRenderDevice.h"
#include "VulkanImpl/VulkanReceipe.h"
#include "VulkanImpl/Conversions.h"
#include "VulkanImpl/VulkanImageView.h"
#include "VulkanImpl/VulkanBuffer.h"

static inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
static inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,void* pUserData);

class App
{
public:
    static constexpr vk::Extent2D WindowDimonsions = { 1280,720 };
    static constexpr std::array<const char*, 1> requiredExt{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };


    struct QueueFamiliesIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentationFamily;

        operator bool() { return graphicsFamily.has_value() && presentationFamily.has_value(); }
    };
    struct SurfaceDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };
    struct Vertex
    {
        glm::vec2 position;
        glm::vec3 color;
        glm::vec2 texCoords;
    };
    struct UniformData
    {
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 model;
    };
public:
    void Run()
    {
        init();
        loop();
        finish();
    }
private:
    void init()
    {
        createWindow();
        initVulkan();
    }
    void loop()
    {
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
            auto currentImage = renderDevice->GetSwapchain()->GetNextImage();

            updateUniformBuffer();

            //Drawing
            {
                vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
                auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffers[currentImage.index])
                                                  .setSignalSemaphoreCount(1).setPSignalSemaphores(&renderFinished)
                                                  .setPWaitDstStageMask(&waitStage);

                device.resetFences(fence);
                queues.graphicsQueue.submit(submitInfo,fence);
                device.waitForFences(fence, true, UINT32_MAX);
            }

            //Presenting
            {
                Receipe* receipe = new VulkanReceipe(renderDevice->getDevice(), { renderFinished, false, nullptr, false });
                renderDevice->GetSwapchain()->Present(receipe);
                delete receipe;
            }

           
        }

        device.waitIdle();
    }
    void finish()
    {
        delete vertexBuffer;
        delete indexBuffer;
        delete matrixUniformBuffer;

        device.destroyImage(image.handle);
        device.destroyImageView(image.view);
        device.freeMemory(image.memory);

        device.destroySampler(sampler);

        device.destroySemaphore(imageAvailable);
        device.destroySemaphore(renderFinished);
        device.destroyFence(fence);

        device.destroyCommandPool(commandPool);

        device.destroyPipeline(pipeline);
        device.destroyPipelineLayout(pipelineLayout);

        device.destroyDescriptorPool(descriptorPool);

        for (const auto& d : descriptorSetLayouts)
            device.destroyDescriptorSetLayout(d);

        for (const auto& f : swapchain.framebuffers)
            device.destroyFramebuffer(f);

        device.destroyRenderPass(renderPass);


        delete renderDevice;;
        renderInstance->getInstance().destroySurfaceKHR(surface);
        delete renderInstance;

        glfwDestroyWindow(window);
        glfwTerminate();
    }
private:
    void createWindow()
    {
        ASSERT(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW");

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WindowDimonsions.width, WindowDimonsions.height, "Valkan Test", nullptr, nullptr);
    }
    void initVulkan()
    {
        createInstance();
        createLogicalDevice();

        createCommandPool();

        createSwapChaine();

        createImage();
        createSampler();

        createRenderPass();
        createFramebuffers();
        createPipeline();
        createUniformBuffers();
        createDescriptorSets();

        createBuffers();

        createCommandBuffer();

        createSyncObjects();
    }
private:
    void createInstance()
    {
        /*uint32_t count;
        const char** glfwExt = glfwGetRequiredInstanceExtensions(&count);
        std::vector<const char*> extensions(glfwExt, glfwExt + count);
        if (ValidationEnabled) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        vk::ApplicationInfo app("Vulkan Test",VK_MAKE_VERSION(1,0,0),
                                "No Engen",VK_MAKE_VERSION(1,0,0),
                                VK_API_VERSION_1_1);

        vk::InstanceCreateInfo createInfo;
        createInfo.pApplicationInfo = &app;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        vk::DebugUtilsMessengerCreateInfoEXT messagerInfo;
        if (ValidationEnabled)
        {
            using Severity = vk::DebugUtilsMessageSeverityFlagBitsEXT;
            using Type = vk::DebugUtilsMessageTypeFlagBitsEXT;

            ASSERT(checkValidationSupport(layers), "Validation layer are not supported");


            messagerInfo.messageSeverity = Severity::eError | Severity::eWarning | Severity::eInfo | Severity::eVerbose;
            messagerInfo.messageType = Type::eGeneral | Type::eValidation | Type::ePerformance;
            messagerInfo.pfnUserCallback = debugCallback;

            createInfo.enabledLayerCount = layers.size();
            createInfo.ppEnabledLayerNames = layers.data();
            createInfo.pNext = &messagerInfo;
        }
        
        instance = vk::createInstance(createInfo);
        LOG_INFO("Instance created successfuly");
        extFunLoader = vk::DispatchLoaderDynamic(instance,vkGetInstanceProcAddr);

        debugMessager = instance.createDebugUtilsMessengerEXT(messagerInfo,nullptr,extFunLoader);
        LOG_INFO("Debug mode enabled");*/

        renderInstance = (VulkanRenderInstance*)(RenderInstance::Create(true));
    }

    void createLogicalDevice()
    {
        renderDevice = (VulkanRenderDevice*)(renderInstance->CreateDevice({window,true,false}));

        device = renderDevice->getDevice();
        LOG_INFO("Logical Device created successfuly");
        queues.graphicsQueue = renderDevice->getGraphicsQueue();
        queues.presentationQueue = renderDevice->getPresentationQueue();
        
        surface = renderDevice->getSurface();
        LOG_INFO("Queues retreived successfuly");
    }
    void createSwapChaine()
    {
        /*
        SurfaceDetails details = getSurfaceDetail(renderDevice->getPhysicalDevice());
        ASSERT(details.formats.size(), "No Format is Suported");
        ASSERT(details.presentModes.size(), "No present mode is Suported");

        vk::Format format;
        vk::ColorSpaceKHR colorSpace;
        {
            bool found = false;
            for (auto& surfaceFormat : details.formats)
            {
                if (surfaceFormat.format == vk::Format::eR8G8B8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
                {
                    format = surfaceFormat.format;
                    colorSpace = surfaceFormat.colorSpace;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                format = details.formats[0].format;
                colorSpace = details.formats[0].colorSpace;
            }
        }

        vk::PresentModeKHR presentMode;
        {
            bool found = false;
            for (auto& mode : details.presentModes)
            {
                if (mode == vk::PresentModeKHR::eMailbox)
                {
                    presentMode = mode;
                    found = true;
                    break;
                }
            }
            if (!found) presentMode = vk::PresentModeKHR::eFifo;
        }

        uint32_t minImageCount;
        {
            minImageCount = details.capabilities.minImageCount + 1;
            if (details.capabilities.maxImageCount != 0)
                minImageCount = std::min(minImageCount, details.capabilities.maxImageCount);
        }

        vk::Extent2D extend;
        {
            if (details.capabilities.currentExtent == vk::Extent2D(-1, -1))
            {
                extend.width = std::clamp(WindowDimonsions.width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
                extend.height = std::clamp(WindowDimonsions.height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
            }
            else
            {
                extend = details.capabilities.currentExtent;
            }
        }

        vk::SharingMode sharingMode;
        std::vector<uint32_t> queues;
        {
            //std::set<uint32_t> s = { renderDevice->m,renderDevice->m_PresentationFamily };
            queues.push_back(0);
            sharingMode = vk::SharingMode::eExclusive;
        }

        vk::SwapchainCreateInfoKHR swapcahainInfo = { {},
            surface,
            minImageCount,
            format,colorSpace,
            extend,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            sharingMode,(uint32_t)queues.size(),queues.data(),
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            presentMode
        };

        swapchain.handle = device.createSwapchainKHR(swapcahainInfo);
        swapchain.format = format;
        swapchain.colorSpace = colorSpace;
        swapchain.extent = extend;
        swapchain.images = device.getSwapchainImagesKHR(swapchain.handle);
        swapchain.views.resize(swapchain.images.size());
        for (uint32_t i = 0; i < swapchain.views.size(); i++)
        {
            swapchain.views[i] = createImageView(swapchain.images[i],vk::ImageViewType::e2D,swapchain.format);
        }
        LOG_INFO("Swapchain created successfuly");
        */
    }
    void createImage()
    {
        uint32_t width = 1, height = 1;
        void* imageData;
        //Image Loading
        {
            int channels,w,h;
            stbi_set_flip_vertically_on_load(true);
            imageData = stbi_load("res/textures/SunSet.jpg", &w, &h, &channels, 4);
            ASSERT(imageData, "Failed to load Image");
            width = w, height = h;
        }

        //Image Creation
        {
            auto imageInfo = vk::ImageCreateInfo()
                .setExtent({ width,height,1 })
                .setImageType(vk::ImageType::e2D)
                .setFormat(vk::Format::eR8G8B8A8Srgb)
                .setMipLevels(1)
                .setArrayLayers(1)
                .setSharingMode(vk::SharingMode::eExclusive)
                .setSamples(vk::SampleCountFlagBits::e1)
                .setTiling(vk::ImageTiling::eOptimal)
                .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled)
                .setInitialLayout(vk::ImageLayout::eUndefined);

            image.handle = device.createImage(imageInfo);
            image.width = width;
            image.height = height;
        }

        //Memory Allocation
        {
            vk::MemoryRequirements reqs = device.getImageMemoryRequirements(image.handle);
            auto allocationInfo = vk::MemoryAllocateInfo()
                .setAllocationSize(reqs.size)
                .setMemoryTypeIndex(findMemoryIndex(reqs.memoryTypeBits,vk::MemoryPropertyFlagBits::eDeviceLocal));

            image.memory = device.allocateMemory(allocationInfo);

        }

        device.bindImageMemory(image.handle, image.memory, 0);

        //Uploading data
        {
            BufferDesc stagingDesc;{
                stagingDesc.usage = BufferUsageBits::TransferSrc | BufferUsageBits::TransferDst;
                stagingDesc.size = image.width * image.height * 4;
                stagingDesc.gpuAccessRate = ResourceAccessRate::Rare;
                stagingDesc.cpuAccessibility = ResourceAccessibilityBits::Write;
            }
            Buffer* stagingBuffer = renderDevice->CreateBuffer(stagingDesc);

            void* ptr = stagingBuffer->Map();
            memcpy(ptr, imageData, image.width * image.height * 4);
            stagingBuffer->UnMap();

            auto barrierInfo = vk::ImageMemoryBarrier()
                .setImage(image.handle)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSubresourceRange({ vk::ImageAspectFlagBits::eColor,0,1,0,1 })
                .setSrcAccessMask({})
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

            auto region = vk::BufferImageCopy()
                .setBufferOffset(0)
                .setBufferImageHeight(0)
                .setBufferRowLength(0)
                .setImageExtent({ (uint32_t)image.width,(uint32_t)image.height,1 })
                .setImageOffset(0)
                .setImageSubresource({ vk::ImageAspectFlagBits::eColor,0,0,1 });

            auto commadAlloc = vk::CommandBufferAllocateInfo()
                .setCommandPool(commandPool)
                .setCommandBufferCount(1)
                .setLevel(vk::CommandBufferLevel::ePrimary);

            vk::CommandBuffer commadBuffer = device.allocateCommandBuffers(commadAlloc)[0];

            commadBuffer.begin({ {vk::CommandBufferUsageFlagBits::eOneTimeSubmit} });
            {
                commadBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrierInfo);
                commadBuffer.copyBufferToImage(static_cast<VulkanBuffer*>(stagingBuffer)->getVkBuffer(), image.handle, vk::ImageLayout::eTransferDstOptimal, region);

                auto barrierInfo2 = vk::ImageMemoryBarrier(barrierInfo)
                    .setOldLayout(vk::ImageLayout::eTransferDstOptimal).setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                    .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite).setDstAccessMask(vk::AccessFlagBits::eShaderRead);

                commadBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrierInfo2);
            }
            commadBuffer.end();

            auto submitInfo = vk::SubmitInfo()
                .setCommandBufferCount(1)
                .setPCommandBuffers(&commadBuffer);

            auto fence = device.createFence({});
            queues.graphicsQueue.submit(submitInfo,fence);

            device.waitForFences(fence, true, (uint64_t)-1);

            device.destroyFence(fence);
            device.freeCommandBuffers(commandPool, commadBuffer);
            delete stagingBuffer;
            stbi_image_free(imageData);

            image.view = createImageView(image.handle, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb);
        }

    }
    void createSampler()
    {
        auto samplerInfo = vk::SamplerCreateInfo()
            .setAddressModeU(vk::SamplerAddressMode::eRepeat).setAddressModeV(vk::SamplerAddressMode::eRepeat).setAddressModeW(vk::SamplerAddressMode::eRepeat)
            .setCompareEnable(false)
            .setAnisotropyEnable(true)
            .setMaxAnisotropy(16)
            .setMagFilter(vk::Filter::eLinear).setMinFilter(vk::Filter::eLinear)
            .setMinLod(0).setMaxLod(0)
            .setMipmapMode(vk::SamplerMipmapMode::eLinear)
            .setUnnormalizedCoordinates(false);

        sampler = device.createSampler(samplerInfo);
    }
    void createCommandPool()
    {
        auto poolInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(0)
            .setFlags(vk::CommandPoolCreateFlagBits::eTransient);
        commandPool = device.createCommandPool(poolInfo);
    }
    void createRenderPass()
    {
        vk::AttachmentReference colorRef = { 0,vk::ImageLayout::eColorAttachmentOptimal };

        std::array<vk::SubpassDescription, 1> subPasses
        {
            vk::SubpassDescription()
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
            .setColorAttachmentCount(1).setPColorAttachments(&colorRef)
        };

        std::array<vk::AttachmentDescription, 1> attachements
        {
            vk::AttachmentDescription()
            .setInitialLayout(vk::ImageLayout::eUndefined).setFinalLayout(vk::ImageLayout::ePresentSrcKHR)
            .setFormat(GetVkFormat(renderDevice->GetSwapchain()->GetImagesDesc().format))
            .setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare),
        };

        std::array<vk::SubpassDependency, 2> depentencies
        {
            vk::SubpassDependency().setSrcSubpass(VK_SUBPASS_EXTERNAL)
                                   .setSrcAccessMask({})
                                   .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            
                                   .setDstSubpass(0)
                                   .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                                   .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput),


             vk::SubpassDependency().setSrcSubpass(0)
                                   .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                                   .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)

                                   .setDstSubpass(VK_SUBPASS_EXTERNAL)
                                   .setDstAccessMask({})
                                   .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput),
        };

        vk::RenderPassCreateInfo passInfo;
        passInfo
            .setAttachmentCount(attachements.size()).setPAttachments(attachements.data())
            .setDependencyCount(depentencies.size()).setPDependencies(depentencies.data())
            .setSubpassCount(subPasses.size()).setPSubpasses(subPasses.data());

        renderPass = device.createRenderPass(passInfo);

        LOG_INFO("RenderPass created successfuly");
    }
    void createFramebuffers()
    {
        swapchain.framebuffers.resize(renderDevice->GetSwapchain()->GetImageCount());
        for (uint32_t i = 0; i < swapchain.framebuffers.size(); i++)
        {
            vk::FramebufferCreateInfo framebufferInfo;
            framebufferInfo
                .setRenderPass(renderPass)
                .setAttachmentCount(1).setPAttachments(&static_cast<VulkanImageView*>(renderDevice->GetSwapchain()->GetImageView(i))->getVkView())
                .setWidth(renderDevice->GetSwapchain()->GetImagesDesc().dimensions.width)
                .setHeight(renderDevice->GetSwapchain()->GetImagesDesc().dimensions.height).setLayers(1);

            swapchain.framebuffers[i] = device.createFramebuffer(framebufferInfo);
        }
    }
    void createPipeline()
    {
        using namespace vk;
        std::array<PipelineShaderStageCreateInfo, 2> stages
        {
            PipelineShaderStageCreateInfo()
                .setModule(createModule("res/shaders/spir-v/shader.vert.spv"))
                .setStage(ShaderStageFlagBits::eVertex)
                .setPName("main"),
            PipelineShaderStageCreateInfo()
                .setModule(createModule("res/shaders/spir-v/shader.frag.spv"))
                .setStage(ShaderStageFlagBits::eFragment)
                .setPName("main")
        };
        PipelineInputAssemblyStateCreateInfo inputAssemblyState;
        {
            inputAssemblyState
                .setTopology(vk::PrimitiveTopology::eTriangleList)
                .setPrimitiveRestartEnable(false);
        }
        PipelineVertexInputStateCreateInfo vertexInputState;
        {
            auto binding = VertexInputBindingDescription().setBinding(0)
                                                          .setInputRate(vk::VertexInputRate::eVertex)
                                                          .setStride(sizeof(Vertex));
            auto attribs = std::array<VertexInputAttributeDescription, 3>({
                    VertexInputAttributeDescription().setBinding(0)
                                                     .setLocation(0)
                                                     .setFormat(vk::Format::eR32G32Sfloat)
                                                     .setOffset(offsetof(Vertex,position)),
                     VertexInputAttributeDescription().setBinding(0)
                                                     .setLocation(1)
                                                     .setFormat(vk::Format::eR32G32B32Sfloat)
                                                     .setOffset(offsetof(Vertex,color)),
                     VertexInputAttributeDescription().setBinding(0)
                                                     .setLocation(2)
                                                     .setFormat(vk::Format::eR32G32Sfloat)
                                                     .setOffset(offsetof(Vertex,texCoords)),

                });

            vertexInputState = PipelineVertexInputStateCreateInfo()
                .setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&binding)
                .setVertexAttributeDescriptionCount(attribs.size()).setPVertexAttributeDescriptions(attribs.data());

        }
        PipelineViewportStateCreateInfo viewportState;
        {
            vk::Viewport viewport = { 0,0,WindowDimonsions.width,WindowDimonsions.height,0,1 };
            vk::Rect2D scissors = { {0,0},{WindowDimonsions.width,WindowDimonsions.height} };
            viewportState
                .setPViewports(&viewport).setViewportCount(1)
                .setPScissors(&scissors).setScissorCount(1);
        }
        PipelineTessellationStateCreateInfo tessellationState;
        {
            
        }
        PipelineDepthStencilStateCreateInfo depthState;
        {
            
        }
        PipelineRasterizationStateCreateInfo resterizationState;
        {
            resterizationState
                .setFrontFace(vk::FrontFace::eClockwise)
                .setCullMode((vk::CullModeFlagBits)0)
                .setDepthBiasEnable(false)
                .setPolygonMode(vk::PolygonMode::eFill)
                .setLineWidth(1.0f);
        }
        PipelineMultisampleStateCreateInfo msState;
        {
            msState
                .setSampleShadingEnable(false)
                .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        }
        PipelineColorBlendStateCreateInfo blendState;
        {
            std::array<PipelineColorBlendAttachmentState, 1> attachements
            {
                PipelineColorBlendAttachmentState()
                    .setBlendEnable(false)
                    .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA),
            };
            blendState
                .setAttachmentCount(attachements.size()).setPAttachments(attachements.data())
                .setLogicOpEnable(false);
        }

        // PipelineLayout
        {
            std::array<DescriptorSetLayoutBinding, 2> bindings =
            {
                DescriptorSetLayoutBinding()
                    .setBinding(0)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eVertex),
                 DescriptorSetLayoutBinding()
                    .setBinding(1)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setDescriptorCount(1)
                    .setStageFlags(vk::ShaderStageFlagBits::eFragment)
            };

            auto descriptorInfo = DescriptorSetLayoutCreateInfo()
                .setBindingCount(bindings.size()).setPBindings(bindings.data());

            descriptorSetLayouts[0] = device.createDescriptorSetLayout(descriptorInfo);

            auto layoutInfo = PipelineLayoutCreateInfo()
                .setSetLayoutCount(descriptorSetLayouts.size()).setPSetLayouts(descriptorSetLayouts.data())
                .setPushConstantRangeCount(0).setPPushConstantRanges(nullptr);

            pipelineLayout = device.createPipelineLayout(layoutInfo);
        }

        GraphicsPipelineCreateInfo pipelineInfo;
        pipelineInfo
            .setPInputAssemblyState(&inputAssemblyState)
            .setPVertexInputState(&vertexInputState)
            .setPViewportState(&viewportState)
            .setPDepthStencilState(nullptr)
            .setPRasterizationState(&resterizationState)
            .setPMultisampleState(&msState)
            .setPColorBlendState(&blendState)
            
            .setLayout(pipelineLayout)

            .setRenderPass(renderPass)
            .setSubpass(0)

            .setPStages(stages.data())
            .setStageCount(stages.size());

        pipeline = device.createGraphicsPipeline({},pipelineInfo).value;

        for (auto& stage : stages)
            device.destroyShaderModule(stage.module);
    }
    void createBuffers()
    {
        BufferDesc desc; {
            desc.usage = BufferUsageBits::VertexBuffer | BufferUsageBits::TransferDst;
            desc.size = sizeof(verteces);
            desc.gpuAccessRate = ResourceAccessRate::Frequent;
            desc.cpuAccessibility = ResourceAccessibilityBits::None;
        }
        vertexBuffer = renderDevice->CreateBuffer(desc);

       {
            desc.usage = BufferUsageBits::IndexBuffer | BufferUsageBits::TransferDst;
            desc.size = sizeof(indeces);
            desc.gpuAccessRate = ResourceAccessRate::Frequent;
            desc.cpuAccessibility = ResourceAccessibilityBits::None;
       }
        indexBuffer = renderDevice->CreateBuffer(desc);

        {
            desc.usage = BufferUsageBits::TransferSrc;
            desc.size = vertexBuffer->GetDesc().size + indexBuffer->GetDesc().size;
            desc.gpuAccessRate = ResourceAccessRate::Rare;
            desc.cpuAccessibility = ResourceAccessibilityBits::Write;
        }
        Buffer* stagingBuffer = renderDevice->CreateBuffer(desc);

        char* ptr = (char*)stagingBuffer->Map();
            memcpy(ptr, verteces.data(), sizeof(verteces));
            memcpy(ptr + vertexBuffer->GetDesc().size, indeces.data(), sizeof(indeces));
        stagingBuffer->UnMap();

        vk::CommandBuffer copyCommand = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo()
            .setCommandBufferCount(1)
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
        )[0];

        copyCommand.begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
        {
            auto region = vk::BufferCopy().setSrcOffset(0)
                                          .setDstOffset(0)
                                          .setSize(vertexBuffer->GetDesc().size);
            copyCommand.copyBuffer(static_cast<VulkanBuffer*>(stagingBuffer)->getVkBuffer(), static_cast<VulkanBuffer*>(vertexBuffer)->getVkBuffer(), region);

            region.setSrcOffset(vertexBuffer->GetDesc().size)
                  .setDstOffset(0)
                  .setSize(indexBuffer->GetDesc().size);
            copyCommand.copyBuffer(static_cast<VulkanBuffer*>(stagingBuffer)->getVkBuffer(), static_cast<VulkanBuffer*>(indexBuffer)->getVkBuffer(), region);
        }
        copyCommand.end();

        auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&copyCommand);
        vk::Fence copyFence = device.createFence({});

        queues.graphicsQueue.submit(submitInfo, copyFence);
        device.waitForFences(copyFence, true, UINT64_MAX);

        delete stagingBuffer;
        device.destroyFence(copyFence);
    }
    void createUniformBuffers()
    {
        BufferDesc desc; {
            desc.usage = BufferUsageBits::UniformBuffer | BufferUsageBits::TransferDst;
            desc.size = sizeof(UniformData);
            desc.gpuAccessRate = ResourceAccessRate::Frequent;
            desc.cpuAccessibility = ResourceAccessibilityBits::Write;
        }
        matrixUniformBuffer = renderDevice->CreateBuffer(desc);
    }
    void createDescriptorSets()
    {
        std::array<vk::DescriptorPoolSize, 2> poolSizes
        {
            vk::DescriptorPoolSize()
            .setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1),
             vk::DescriptorPoolSize()
            .setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(1),
        };

        auto poolInfo = vk::DescriptorPoolCreateInfo()
            .setPoolSizeCount(poolSizes.size()).setPPoolSizes(poolSizes.data())
            .setMaxSets(1);

        descriptorPool = device.createDescriptorPool(poolInfo);

        auto allocInfo = vk::DescriptorSetAllocateInfo()
            .setDescriptorPool(descriptorPool)
            .setDescriptorSetCount(1)
            .setPSetLayouts(&descriptorSetLayouts[0]);

        auto sets = device.allocateDescriptorSets(allocInfo);
        matrixSet = sets[0];

        auto bufferInfo = vk::DescriptorBufferInfo()
            .setBuffer(static_cast<VulkanBuffer*>(matrixUniformBuffer)->getVkBuffer())
            .setOffset(0)
            .setRange(matrixUniformBuffer->GetDesc().size);

        auto textureInfo = vk::DescriptorImageInfo()
            .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
            .setImageView(image.view)
            .setSampler(sampler);

        std::array< vk::WriteDescriptorSet, 2> writeInfos =
        {
            vk::WriteDescriptorSet()
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                .setDstSet(matrixSet)
                .setDstBinding(0)
                .setDstArrayElement(0)
                .setPBufferInfo(&bufferInfo),
            vk::WriteDescriptorSet()
                .setDescriptorCount(1)
                .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                .setDstSet(matrixSet)
                .setDstBinding(1)
                .setDstArrayElement(0)
                .setPImageInfo(&textureInfo)
        };
        device.updateDescriptorSets(writeInfos, nullptr);
    }
    void createCommandBuffer()
    {
        auto commandAllocInfo = vk::CommandBufferAllocateInfo().setCommandBufferCount(renderDevice->GetSwapchain()->GetImageCount())
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary);

        commandBuffers = device.allocateCommandBuffers(commandAllocInfo);

        auto clearColor = vk::ClearValue().setColor(std::array<float, 4>{0.2,0.3,0.8,1});
        auto beginInfo = vk::CommandBufferBeginInfo();
        uint32_t i = 0;
        for (auto& commandBuffer : commandBuffers)
        {
            auto renderPassBeginInfo = vk::RenderPassBeginInfo()
                .setFramebuffer(swapchain.framebuffers[i])
                .setClearValueCount(1).setPClearValues(&clearColor)
                .setRenderPass(renderPass)
                .setRenderArea({ {0,0},{renderDevice->GetSwapchain()->GetImagesDesc().dimensions.width,renderDevice->GetSwapchain()->GetImagesDesc().dimensions.height} });

            commandBuffer.begin(beginInfo);
            {
                commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
                {
                    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
                    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, matrixSet, {});

                    commandBuffer.bindVertexBuffers(0, static_cast<VulkanBuffer*>(vertexBuffer)->getVkBuffer(), (vk::DeviceSize)0);
                    commandBuffer.bindIndexBuffer(static_cast<VulkanBuffer*>(indexBuffer)->getVkBuffer(), 0, vk::IndexType::eUint32);

                    commandBuffer.drawIndexed(indeces.size(), 1, 0, 0, 0);
                }
                commandBuffer.endRenderPass();
            }
            commandBuffer.end();

            i++;
        }
    }
    void createSyncObjects()
    {
        imageAvailable = device.createSemaphore({});
        renderFinished = device.createSemaphore({});
        fence = device.createFence({});
    }

    void updateUniformBuffer()
    {
        currentRotation += 0.016f * glm::radians(90.f);
        UniformData data;

        data.proj = glm::perspective(glm::radians(70.f), WindowDimonsions.width / (float)WindowDimonsions.height, 0.1f, 1000.f);
        data.view = glm::lookAt(glm::vec3{ 0,0,0 }, glm::vec3{ 0,0,-1 }, glm::vec3{ 0,1,0 });
        data.model = glm::translate(glm::mat4(1), { 0,0,-2 }) * glm::rotate(glm::mat4(1), currentRotation, {0,1,0}) *
            glm::scale(glm::mat4(1), {1,1,1});

        void* memory = matrixUniformBuffer->Map();
        memcpy(memory, &data, sizeof(data));
        matrixUniformBuffer->UnMap();

    }

    template<size_t N>
    inline  bool checkValidationSupport(const std::array<const char*, N>& layers)
    {
        auto props = vk::enumerateInstanceLayerProperties();

        for (auto& layer : layers)
        {
            bool found = false;
            for (auto& prop : props)
            {
                if (strcmp(layer, prop.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }

        return true;
    }
    QueueFamiliesIndices getFamilies(const vk::PhysicalDevice& device)
    {
        std::vector<vk::QueueFamilyProperties> families = device.getQueueFamilyProperties();

        QueueFamiliesIndices indeces;
        uint32_t index = 0;
        for (auto& family : families)
        {
            if (family.queueFlags & vk::QueueFlagBits::eGraphics)
                indeces.graphicsFamily = index;
            if (device.getSurfaceSupportKHR(index, surface))
                indeces.presentationFamily = index;

            if (indeces)
                break;
            index++;
        }

        return indeces;
    }
    inline SurfaceDetails getSurfaceDetail(const vk::PhysicalDevice& device)
    {
        return {
            device.getSurfaceCapabilitiesKHR(surface),
            device.getSurfaceFormatsKHR(surface),
            device.getSurfacePresentModesKHR(surface)
        };
    }
    inline vk::ImageView createImageView(vk::Image image, vk::ImageViewType type,vk::Format format)
    {
        vk::ImageViewCreateInfo viewInfo;

        viewInfo.setImage(image)
            .setFormat(format)
            .setViewType(type)
            .setSubresourceRange({ vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });

        return device.createImageView(viewInfo);
    }
    vk::ShaderModule createModule(const char* path)
    {
        vk::ShaderModuleCreateInfo moduleInfo;
        std::optional<std::vector<char>> oData = loadFile(path);
        ASSERT(oData.has_value(), "Failed to create a Module");

        std::vector<char>& data = oData.value();

        moduleInfo
            .setCodeSize(data.size())
            .setPCode((uint32_t*)data.data());
        
        return device.createShaderModule(moduleInfo);
    }
    std::optional<std::vector<char>> loadFile(const char* path)
    {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (file)
        {
            std::vector<char> data(file.tellg());
            file.seekg(0);
            file.read(data.data(), data.size());
            return data;
        }
        LOG_WARN("Could not find \"%s\"", path);
        return {};
    }
   
    uint32_t findMemoryIndex(uint32_t suitable, vk::MemoryPropertyFlags memoryProperties)
    {
        vk::PhysicalDeviceMemoryProperties deviceMemoryProps = renderDevice->getPhysicalDevice().getMemoryProperties();

        for (uint32_t i = 0; i < deviceMemoryProps.memoryTypeCount; i++)
        {
            if ((suitable & (1 << i)) && ((deviceMemoryProps.memoryTypes[i].propertyFlags & memoryProperties) == memoryProperties))
            {
                return i;
            }
        }

        ASSERT(false, "Could not find the requested memory");
    }
private:
    GLFWwindow* window;

    VulkanRenderInstance* renderInstance;
    VulkanRenderDevice* renderDevice;

    vk::Device device;

    vk::SurfaceKHR surface;

    struct 
    {
        std::vector<vk::Framebuffer> framebuffers;
    } swapchain;

    struct 
    {
        vk::Queue graphicsQueue; 
        vk::Queue presentationQueue;
    } queues;

    struct
    {
        vk::Image handle;
        vk::ImageView view;
        vk::DeviceMemory memory;

        vk::DeviceSize width, height;
    } image;

    vk::Sampler sampler;

    vk::RenderPass renderPass;
    vk::Pipeline pipeline;
    vk::PipelineLayout pipelineLayout;
    std::array<vk::DescriptorSetLayout,1> descriptorSetLayouts;

    vk::DescriptorPool descriptorPool;
    vk::DescriptorSet matrixSet;
    vk::DescriptorSet textureSet;
    Buffer* matrixUniformBuffer;

    vk::CommandPool commandPool;
    std::vector<vk::CommandBuffer> commandBuffers;

    Buffer* vertexBuffer;
    std::array<Vertex, 4> verteces{ {
        {{-0.5,-0.5},{ 1  , 0  , 0  },{0,1}},
        {{ 0.5,-0.5},{ 1  , 1  , 1  },{1,1}},
        {{ 0.5, 0.5},{ 0  , 1  , 0  },{1,0}},
        {{-0.5, 0.5},{ 0  , 0  , 1  },{0,0}},
    } };
    Buffer* indexBuffer;
    std::array<uint32_t, 6> indeces{
        0,1,2,
        2,3,0
    };


    vk::Semaphore imageAvailable, renderFinished;
    vk::Fence fence;
    
    
        float currentRotation = 0;
    #ifdef VALIDATION_LAYERS
        vk::DebugUtilsMessengerEXT debugMessager;
    #endif

};

int main()
{
    App app;
    app.Run();
}

static inline VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    ASSERT(func, "Failed to load CreateDebugUtilsMessengerEXT");
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
}
static inline void DestroyDebugUtilsMessengerEXT(VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    ASSERT(func, "Failed to load CreateDebugUtilsMessengerEXT");
    return func(instance, debugMessenger,pAllocator);
}
 VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
     constexpr vk::DebugUtilsMessageSeverityFlagBitsEXT severity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning;

     if (messageSeverity >= (VkDebugUtilsMessageSeverityFlagBitsEXT)severity)
     {
         switch (messageSeverity)
         {
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
             LOG_TRACE("[Validation layer] Verbose : %s \n%s\n",pCallbackData->pMessageIdName, pCallbackData->pMessage);
             break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
             LOG_INFO("[Validation layer] Info : %s \n%s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
             break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
             LOG_WARN("[Validation layer] Warning : %s \n%s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
             break;
         case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
             ASSERT(false,"[Validation layer] ERROR : %s \n%s\n", pCallbackData->pMessageIdName, pCallbackData->pMessage);
             break;
         default:
             break;
         }
     }

    return VK_FALSE;
}
