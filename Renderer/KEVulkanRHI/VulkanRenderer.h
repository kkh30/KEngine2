#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H
#include "VulkanCore.h"
#include "KEModule.h"
#include "VulkanRenderTarget.h"
#include "VulkanFrameBuffer.h"
#include "VulkanStaticBuffer.h"

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
	virtual void Update() override;
	void InitVMAllocator();
	void InitSwapChain();
	void InitResources();
	void InitFramebuffers();
	void InitDepthStencilBuffer();
	void InitDeferredPass();
	void InitPipelineLayout();
	void InitPipelineState();
	void InitCmdPool();
	void InitCmdBuffers();
	void RecordCmdBuffers();
	void InitDateBuffers();

	void InitSynchronizationPrimitives();

private:
	VkRenderPass m_deferred_pass = VK_NULL_HANDLE;
	bool VK_KHR_get_memory_requirements2_enabled = true;
	bool VK_KHR_dedicated_allocation_enabled = true;
	VulkanSwapChain m_swap_chain = VulkanSwapChain();
	KEVulkanRenderTarget* m_depth_stencil_buffer = nullptr;
	VkFormat m_depth_format = VK_FORMAT_UNDEFINED;
	std::vector<KEVulkanFramebuffer> m_framebuffers = {};
	std::vector<VkCommandBuffer> m_cmd_buffers = {};
	struct Synchronization
	{
		VkSemaphore presentCompleteSemaphore;
		VkSemaphore renderCompleteSemaphore;
	}m_semaphores = {};
	std::vector<VkFence> m_fences;
	uint32_t m_currentBuffer = 0;
	KEVulkanStaticBuffer m_vertex_buffer;
};






#endif // !VULKAN_RENDERER_H
