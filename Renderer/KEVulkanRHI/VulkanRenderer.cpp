#include "VulkanRenderer.h"




KEVulkanRenderer::KEVulkanRenderer(
	const KEVulkanRendererDescriptor& p_renderer_desc,
	KEWindow* target_window
	)
{
	// Create a Vulkan Instance
	VulkanCore::instance = new CInstance(true);
	VulkanCore::instance->DebugReport.SetFlags(VulkanCore::debug_repost_flags);
	VulkanCore::surface = target_window->GetSurface(*VulkanCore::instance);
	VulkanCore::gpus = new CPhysicalDevices(*VulkanCore::instance);
	//VulkanCore::gpus->Print(true);
	VulkanCore::gpu = VulkanCore::gpus->FindPresentable(VulkanCore::surface);
	VulkanCore::device = new CDevice(*VulkanCore::gpu);
	VulkanCore::device->gpu.extensions.Pick("VK_KHR_get_memory_requirements2");
	VulkanCore::device->gpu.extensions.Pick("VK_KHR_dedicated_allocation");
	VulkanCore::graphics_queue = VulkanCore::device->AddQueue(VK_QUEUE_GRAPHICS_BIT, VulkanCore::surface);
	VulkanCore::vk_device = *VulkanCore::device;
	m_depth_format = VulkanCore::FindDepthFormat();

}

KEVulkanRenderer::~KEVulkanRenderer()
{
}

void KEVulkanRenderer::OnStartUp()
{
	InitVMAllocator();
	InitSwapChain();
	InitDeferredPass();
	InitResources();
	InitCmdPool();
	InitCmdBuffers();
	RecordCmdBuffers();
	InitSynchronizationPrimitives();
}



void KEVulkanRenderer::OnShutDown()
{

	vmaDestroyAllocator(VulkanCore::allocator);
	VulkanCore::allocator = nullptr;
	if (VulkanCore::graphics_queue) {
		delete VulkanCore::graphics_queue;
		VulkanCore::graphics_queue = nullptr;
	}

	if (VulkanCore::gpus) {
		delete VulkanCore::gpus;
		VulkanCore::gpus = nullptr;
	}

	if (VulkanCore::instance) {
		delete VulkanCore::instance;
		VulkanCore::instance = nullptr;
	}
}

void KEVulkanRenderer::Update()
{
	// Get next image in the swap chain (back/front buffer)
	m_swap_chain.acquireNextImage(m_semaphores.presentCompleteSemaphore, &m_currentBuffer);

	// m_currentBuffer = (m_currentBuffer + 1) % 3;
	// Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
	vkWaitForFences(VulkanCore::vk_device, 1, &m_fences[m_currentBuffer], VK_TRUE, UINT64_MAX);
	vkResetFences(VulkanCore::vk_device, 1, &m_fences[m_currentBuffer]);

	VkPipelineStageFlags waitShadowStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	// The submit info structure specifices a command buffer queue submission batch
	VkSubmitInfo l_subinfo = {};
	l_subinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	l_subinfo.pWaitDstStageMask = &waitShadowStageMask;									// Pointer to the list of pipeline stages that the semaphore waits will occur at
	l_subinfo.pWaitSemaphores = &m_semaphores.presentCompleteSemaphore;							// Semaphore(s) to wait upon before the submitted command buffer starts executing
	l_subinfo.waitSemaphoreCount = 1;												// One wait semaphore																				
	l_subinfo.pSignalSemaphores = &m_semaphores.renderCompleteSemaphore;						// Semaphore(s) to be signaled when command buffers have completed
	l_subinfo.signalSemaphoreCount = 1;											// One signal semaphore
	l_subinfo.pCommandBuffers = &m_cmd_buffers[m_currentBuffer];					// Command buffers(s) to execute in this batch (submission)
	l_subinfo.commandBufferCount = 1;
	
	// Submit to the graphics queue passing a wait fence
	vkQueueSubmit(*VulkanCore::graphics_queue, 1, &l_subinfo, m_fences[m_currentBuffer]);
	//(vkQueueSubmit(m_graphics_queue, 1, &submitInfo, nullptr));

	// Present the current buffer to the swap chain
	// Pass the semaphore signaled by the command buffer submission from the submit info as the wait semaphore for swap chain presentation
	// This ensures that the image is not presented to the windowing system until all commands have been submitted
	m_swap_chain.queuePresent(*VulkanCore::graphics_queue, m_currentBuffer, m_semaphores.renderCompleteSemaphore);

}

void KEVulkanRenderer::RecordCmdBuffers()
{
	VkCommandBufferBeginInfo l_begin_info = {};
	l_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.1f, 0.6f, 0.3f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = m_deferred_pass;
	renderPassBeginInfo.renderArea = { {0,0},{ KEWindow::Instance().GetWidth(),KEWindow::Instance().GetHeight() } };
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;
	int i = 0;
	for (auto& cmd : m_cmd_buffers) {
		vkBeginCommandBuffer(cmd, &l_begin_info);
		//Record Drawcalls.
		{
			renderPassBeginInfo.framebuffer = m_framebuffers[i++];
			vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			// Update dynamic viewport state
			VkViewport viewport = {};
			viewport.width =  (float)KEWindow::Instance().GetWidth();
			viewport.height = (float)KEWindow::Instance().GetHeight();
			viewport.minDepth = (float) 0.0f;
			viewport.maxDepth = (float) 1.0f;
			vkCmdSetViewport(cmd, 0, 1, &viewport);

			// Update dynamic scissor state
			VkRect2D scissor = {};
			scissor.extent.width  = KEWindow::Instance().GetWidth();
			scissor.extent.height = KEWindow::Instance().GetHeight();
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(cmd, 0, 1, &scissor);
			vkCmdEndRenderPass(cmd);
		}
		vkEndCommandBuffer(cmd);
	}
}

void KEVulkanRenderer::InitSynchronizationPrimitives()
{
	// Semaphores (Used for correct command ordering)
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;

	// Semaphore used to ensures that image presentation is complete before starting to submit again
	(vkCreateSemaphore(VulkanCore::vk_device, &semaphoreCreateInfo, nullptr, &m_semaphores.presentCompleteSemaphore));

	// Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
	(vkCreateSemaphore(VulkanCore::vk_device, &semaphoreCreateInfo, nullptr, &m_semaphores.renderCompleteSemaphore));

	// Fences (Used to check draw command buffer completion)
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	// Create in signaled state so we don't wait on first render of each command buffer
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	m_fences.resize(m_swap_chain.imageCount);
	for (auto& fence : m_fences)
	{
		vkCreateFence(VulkanCore::vk_device, &fenceCreateInfo, nullptr, &fence);
	}
	
}


void KEVulkanRenderer::InitVMAllocator()
{
	// Create memory allocator

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = VulkanCore::gpu->handle;
	allocatorInfo.device = VulkanCore::vk_device;

	if (VK_KHR_dedicated_allocation_enabled)
	{
		allocatorInfo.flags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
	}

	VkAllocationCallbacks cpuAllocationCallbacks = {};
	//if (USE_CUSTOM_CPU_ALLOCATION_CALLBACKS)
	//{
	//	cpuAllocationCallbacks.pUserData = CUSTOM_CPU_ALLOCATION_CALLBACK_USER_DATA;
	//	cpuAllocationCallbacks.pfnAllocation = &CustomCpuAllocation;
	//	cpuAllocationCallbacks.pfnReallocation = &CustomCpuReallocation;
	//	cpuAllocationCallbacks.pfnFree = &CustomCpuFree;
	//	allocatorInfo.pAllocationCallbacks = &cpuAllocationCallbacks;
	//}

	vmaCreateAllocator(&allocatorInfo, &VulkanCore::allocator);
}

void KEVulkanRenderer::InitSwapChain()
{
	m_swap_chain.connect(*VulkanCore::instance, *VulkanCore::gpu, *VulkanCore::device);
	m_swap_chain.initVKSurfaceWithSystemSurface(VulkanCore::surface);
	m_swap_chain.create(KEWindow::Instance().GetWidth(), KEWindow::Instance().GetHeight());
}

void KEVulkanRenderer::InitResources()
{
	InitDepthStencilBuffer();
	InitFramebuffers();
}

void KEVulkanRenderer::InitFramebuffers()
{
	m_framebuffers.resize(m_swap_chain.imageCount);
	int i = 0;
	for (auto& framebuffer : m_framebuffers) {
		framebuffer.AddAttachment(m_swap_chain.buffers[i].view);
		framebuffer.AddAttachment(m_depth_stencil_buffer->GetImageView());
		framebuffer.CreateFramebuffer(KEWindow::Instance().GetWidth(), KEWindow::Instance().GetHeight(), m_deferred_pass);
		i++;
	}
}

void KEVulkanRenderer::InitDepthStencilBuffer()
{
	    m_depth_stencil_buffer = new KEVulkanRenderTarget(
		KEWindow::Instance().GetWidth(), 
		KEWindow::Instance().GetHeight(),
		m_depth_format,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY);
}

void KEVulkanRenderer::InitDeferredPass()
{
	//Init Attachments
	VkAttachmentDescription l_color_attachment0 = {};
	VkAttachmentReference l_color_attachment0_ref = {};
	{
		//Attachement
		l_color_attachment0.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		l_color_attachment0.format = m_swap_chain.colorFormat;
		l_color_attachment0.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		l_color_attachment0.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		l_color_attachment0.samples = VK_SAMPLE_COUNT_1_BIT;
		l_color_attachment0.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		l_color_attachment0.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		l_color_attachment0.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		//Reference
		l_color_attachment0_ref.attachment = 0;
		l_color_attachment0_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	
	VkAttachmentDescription l_depth_stencil = {};
	VkAttachmentReference l_depth_stencil_ref = {};
	{
		//Attachment
		l_depth_stencil.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		l_depth_stencil.format = m_depth_format;
		l_depth_stencil.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		l_depth_stencil.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		l_depth_stencil.samples = VK_SAMPLE_COUNT_1_BIT;
		l_depth_stencil.stencilLoadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		l_depth_stencil.stencilStoreOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		l_depth_stencil.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_DONT_CARE;
		//Reference
		l_depth_stencil_ref.attachment = 1;
		l_depth_stencil_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	std::vector<VkAttachmentDescription> l_attachments = { l_color_attachment0 ,l_depth_stencil };
	std::vector<VkAttachmentReference> l_attachments_ref = { l_color_attachment0_ref};

	//Init Dependencies
	VkSubpassDependency l_externl_to_render_pass_dependency = {};
	{
		l_externl_to_render_pass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		l_externl_to_render_pass_dependency.dstSubpass = 0;
		l_externl_to_render_pass_dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		l_externl_to_render_pass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		l_externl_to_render_pass_dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		l_externl_to_render_pass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		l_externl_to_render_pass_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	}
	VkSubpassDependency l_render_pass_to_external_dependency = {};
	{
		l_render_pass_to_external_dependency.srcSubpass = 0;
		l_render_pass_to_external_dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
		l_render_pass_to_external_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		l_render_pass_to_external_dependency.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		l_render_pass_to_external_dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		l_render_pass_to_external_dependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		l_render_pass_to_external_dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	}
	std::vector<VkSubpassDependency> l_dependencies = { l_externl_to_render_pass_dependency ,l_render_pass_to_external_dependency };
	
	//Init Subpasses
	VkSubpassDescription l_geometry_subpass = {};
	l_geometry_subpass.colorAttachmentCount = l_attachments_ref.size();
	l_geometry_subpass.pColorAttachments = l_attachments_ref.data();
	l_geometry_subpass.pDepthStencilAttachment = &l_depth_stencil_ref;
	std::vector<VkSubpassDescription> l_subpasses = { l_geometry_subpass };

	VkRenderPassCreateInfo l_renderpass_create_info = {};
	l_renderpass_create_info.attachmentCount = l_attachments.size();
	l_renderpass_create_info.pAttachments = l_attachments.data();
	l_renderpass_create_info.pDependencies = l_dependencies.data();
	l_renderpass_create_info.dependencyCount = l_dependencies.size();
	l_renderpass_create_info.pSubpasses = l_subpasses.data();
	l_renderpass_create_info.subpassCount = l_subpasses.size();
	l_renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkCreateRenderPass(VulkanCore::vk_device, &l_renderpass_create_info, nullptr, &m_deferred_pass);
}

void KEVulkanRenderer::InitPipelineLayout()
{
}

void KEVulkanRenderer::InitPipelineState()
{
}

void KEVulkanRenderer::InitCmdPool()
{
	VkCommandPoolCreateInfo l_create_info = {};
	l_create_info.queueFamilyIndex = VulkanCore::graphics_queue->index;
	l_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	l_create_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	vkCreateCommandPool(VulkanCore::vk_device, &l_create_info, nullptr, &VulkanCore::cmd_pool);
}

void KEVulkanRenderer::InitCmdBuffers()
{
	VkCommandBufferAllocateInfo l_create_info = {};
	l_create_info.commandBufferCount = m_swap_chain.imageCount;
	l_create_info.commandPool = VulkanCore::cmd_pool;
	l_create_info.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	l_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	m_cmd_buffers.resize(m_swap_chain.imageCount);
	vkAllocateCommandBuffers(VulkanCore::vk_device, &l_create_info, m_cmd_buffers.data());
}

