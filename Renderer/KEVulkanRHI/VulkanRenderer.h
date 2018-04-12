#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H
#define VMA_USE_STL_CONTAINERS
#include "vk_mem_alloc.h"
#include <vulkan\vulkan.h>
#include "KEModule.h"
#include "WSIWindow\CDevices.h"
#include "KEWindow.h"
#include "VulkanSwapChain.h"


struct KEVulkanRendererDescriptor
{
	bool enable_validation = false;
	
};

class KEVulkanRenderer final: public Module<KEVulkanRenderer>
{
public:
	KEVulkanRenderer
	(
		const KEVulkanRendererDescriptor& p_renderer_desc,
		KEWindow* target_window
	);
	~KEVulkanRenderer();

	virtual void OnStartUp() override;
	virtual void OnShutDown() override;
	void InitVMAllocator();
	void InitSwapChain();
	void InitResources();
	void InitFramebuffers();
	void InitDepthStencilBuffer();
private:
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	CInstance* m_instance = nullptr;
	CPhysicalDevices* m_gpus = nullptr;
	CPhysicalDevice *m_gpu = nullptr;
	CDevice* m_device = nullptr;
	CQueue* m_graphics_queue = nullptr;
	bool VK_KHR_get_memory_requirements2_enabled = true;
	bool VK_KHR_dedicated_allocation_enabled = true;
	VmaAllocator m_allocator;
	VulkanSwapChain m_swap_chain = VulkanSwapChain();

};






#endif // !VULKAN_RENDERER_H
