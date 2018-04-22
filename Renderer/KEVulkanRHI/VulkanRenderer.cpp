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
	InitCmdPool();
	InitCmdBuffers();
	InitResources();
	InitPipelines();
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
	// Pass the semaphore signaled by the command buf fer submission from the submit info as the wait semaphore for swap chain presentation
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
			//vkdeviceoffset
			VkDeviceSize offsets = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, *m_vertex_buffer, &offsets);
			vkCmdBindIndexBuffer(cmd, *m_index_buffer, offsets, VkIndexType::VK_INDEX_TYPE_UINT32);
			vkCmdBindPipeline(cmd, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
			vkCmdDrawIndexed(cmd, 3, 1, 0, 0, 0);
			vkCmdEndRenderPass(cmd);
		}
		vkEndCommandBuffer(cmd);
	}
}

void KEVulkanRenderer::InitDateBuffers()
{
	// Setup vertices
	std::vector<float> vertexBuffer =
	{
		1.0f,  1.0f, 0.0f ,1.0f, 0.0f, 0.0f ,
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, -1.0f, 0.0f ,0.0f, 0.0f, 1.0f ,
	};
	uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0, 1, 2 };

	//Init Vertex Buffer
	m_vertex_buffer = new KEVulkanVertexBuffer(vertexBuffer);
	//Init Index Buffer
	m_index_buffer = new KEVulkanIndexBuffer(indexBuffer);
}

void KEVulkanRenderer::InitPipelines()
{
	//Init PipelineLayout
	{
		VkPipelineLayoutCreateInfo l_create_info = {};
		l_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		vkCreatePipelineLayout(VulkanCore::vk_device, &l_create_info, nullptr, &m_pipeline_layout);
	}
	//Init PipelineCaceh
	{
		VkPipelineCacheCreateInfo l_create_info = {};
		l_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		vkCreatePipelineCache(VulkanCore::vk_device, &l_create_info, nullptr, &m_pipeline_cache);
	}
	// Create the graphics pipeline used in this example
	// Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
	// A pipeline is then stored and hashed on the GPU making pipeline changes very fast
	// Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
	pipelineCreateInfo.layout = m_pipeline_layout;
	// Renderpass this pipeline is attached to
	pipelineCreateInfo.renderPass = m_deferred_pass;

	// Construct the differnent states making up the pipeline

	// Input assembly state describes how primitives are assembled
	// This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	// Color blend state describes how blend factors are calculated (if used)
	// We need one blend attachment state per color attachment (even if blending is not used
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = VK_FALSE;
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = blendAttachmentState;

	// Viewport state sets the number of viewports and scissor used in this pipeline
	// Note: This is actually overriden by the dynamic states (see below)
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// Enable dynamic states
	// Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
	// To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
	// For this example we will set the viewport and scissor using dynamic states
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Depth and stencil state containing depth and stencil compare and test operations
	// We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = depthStencilState.back;

	// Multi sampling state
	// This example does not make use fo multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.pSampleMask = nullptr;

	// Vertex input descriptions 
	// Specifies the vertex input parameters for a pipeline

	// Vertex input binding
	// This example uses a single vertex input binding at binding point 0 (see vkCmdBindVertexBuffers)
	VkVertexInputBindingDescription vertexInputBinding = {};
	vertexInputBinding.binding = 0;
	vertexInputBinding.stride = sizeof(Vertex);
	vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Inpute attribute bindings describe shader attribute locations and memory layouts
	std::array<VkVertexInputAttributeDescription, 2> vertexInputAttributs;
	// These match the following shader layout (see triangle.vert):
	//	layout (location = 0) in vec3 inPos;
	//	layout (location = 1) in vec3 inColor;
	// Attribute location 0: Position
	vertexInputAttributs[0].binding = 0;
	vertexInputAttributs[0].location = 0;
	// Position attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[0].offset = offsetof(Vertex, position);
	// Attribute location 1: Color
	vertexInputAttributs[1].binding = 0;
	vertexInputAttributs[1].location = 1;
	// Color attribute is three 32 bit signed (SFLOAT) floats (R32 G32 B32)
	vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributs[1].offset = offsetof(Vertex, color);

	// Vertex input state used for pipeline creation
	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.vertexBindingDescriptionCount = 1;
	vertexInputState.pVertexBindingDescriptions = &vertexInputBinding;
	vertexInputState.vertexAttributeDescriptionCount = 2;
	vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs.data();

	// Shaders
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages{};

	// Vertex shader
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	// Set pipeline stage for this shader
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	// Load binary SPIR-V shader
	shaderStages[0].module = VulkanCore::loadSPIRVShader("D:\\Dev\\Vulkan\\data\\shaders\\triangle\\triangle.vert.spv");
	// Main entry point for the shader
	shaderStages[0].pName = "main";
	assert(shaderStages[0].module != VK_NULL_HANDLE);

	// Fragment shader
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	// Set pipeline stage for this shader
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	// Load binary SPIR-V shader
	shaderStages[1].module = VulkanCore::loadSPIRVShader("D:\\Dev\\Vulkan\\data\\shaders\\triangle\\triangle.frag.spv");
	// Main entry point for the shader
	shaderStages[1].pName = "main";
	assert(shaderStages[1].module != VK_NULL_HANDLE);

	// Set pipeline shader stage info
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	// Assign the pipeline states to the pipeline creation info structure
	pipelineCreateInfo.pVertexInputState = &vertexInputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.renderPass = m_deferred_pass;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	// Create rendering pipeline using the specified states
	vkCreateGraphicsPipelines(VulkanCore::vk_device, m_pipeline_cache, 1, &pipelineCreateInfo, nullptr, &m_pipeline);

	// Shader modules are no longer needed once the graphics pipeline has been created
	VulkanCore::DestroyShaderModule(shaderStages[0].module);
	VulkanCore::DestroyShaderModule(shaderStages[1].module);
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
	InitDateBuffers();
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


void KEVulkanRenderer::InitCmdPool()
{
	VkCommandPoolCreateInfo l_create_info = {};
	l_create_info.queueFamilyIndex = VulkanCore::graphics_queue->index;
	l_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	l_create_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT| VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
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

	//Allocate A temp cmd buffer for resources allocation.
	l_create_info.commandBufferCount = 1;
	vkAllocateCommandBuffers(VulkanCore::vk_device, &l_create_info, &VulkanCore::temp_command_buffer);

}

