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

struct Vertex {
	float position[3];
	float color[3];
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
	void InitCmdPool();
	void InitCmdBuffers();
	void RecordCmdBuffers();
	void InitDateBuffers();
	void InitPipelines();
	void InitSynchronizationPrimitives();

private:
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	VkPipelineLayout m_pipeline_layout = VK_NULL_HANDLE;
	VkPipelineCache m_pipeline_cache = VK_NULL_HANDLE;
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
	KEVulkanVertexBuffer* m_vertex_buffer = nullptr;
	KEVulkanIndexBuffer* m_index_buffer = nullptr;

};






#endif // !VULKAN_RENDERER_H
