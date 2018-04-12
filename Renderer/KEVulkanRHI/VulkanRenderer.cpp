#include "VulkanRenderer.h"




KEVulkanRenderer::KEVulkanRenderer(
	const KEVulkanRendererDescriptor& p_renderer_desc,
	KEWindow* target_window
	)
{
	// Create a Vulkan Instance
	m_instance = new CInstance(true);
	m_instance->DebugReport.SetFlags(14);
	m_surface = target_window->GetSurface(*m_instance);
	m_gpus = new CPhysicalDevices(*m_instance);
	m_gpus->Print(true);
	m_gpu = m_gpus->FindPresentable(m_surface);
	m_device = new CDevice(*m_gpu);
	m_graphics_queue = m_device->AddQueue(VK_QUEUE_GRAPHICS_BIT, m_surface);
}

KEVulkanRenderer::~KEVulkanRenderer()
{
}

void KEVulkanRenderer::OnStartUp()
{
	InitVMAllocator();
	InitSwapChain();
	InitResources();

}

void KEVulkanRenderer::OnShutDown()
{

	vmaDestroyAllocator(m_allocator);
	m_allocator = nullptr;
	if (m_graphics_queue) {
		delete m_graphics_queue;
		m_graphics_queue = nullptr;
	}

	if (m_gpus) {
		delete m_gpus;
		m_gpus = nullptr;
	}

	if (m_instance) {
		delete m_instance;
		m_instance = nullptr;
	}
}

void KEVulkanRenderer::InitVMAllocator()
{
	// Create memory allocator

	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_gpu->handle;
	allocatorInfo.device = m_device->handle;

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

	vmaCreateAllocator(&allocatorInfo, &m_allocator);
}

void KEVulkanRenderer::InitSwapChain()
{
	m_swap_chain.connect(*m_instance, *m_gpu, *m_device);
	m_swap_chain.initVKSurfaceWithSystemSurface(m_surface);
	m_swap_chain.create(KEWindow::Instance().GetWidth(), KEWindow::Instance().GetHeight());
}

void KEVulkanRenderer::InitResources()
{
	InitFramebuffers();
	InitDepthStencilBuffer();
}

void KEVulkanRenderer::InitFramebuffers()
{
}

void KEVulkanRenderer::InitDepthStencilBuffer()
{
}
