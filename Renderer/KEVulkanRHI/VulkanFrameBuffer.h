#pragma once
#include "VulkanRenderTarget.h"
#include <vector>


class KEVulkanFramebuffer
{
public:
	KEVulkanFramebuffer::KEVulkanFramebuffer():
		m_framebuffer(VK_NULL_HANDLE),
		m_attachment()

	{
		
	}

	KEVulkanFramebuffer::~KEVulkanFramebuffer()
	{
	}

	void CreateFramebuffer(const uint32_t& p_width,const uint32_t& p_height,VkRenderPass p_render_pass) {
		VkFramebufferCreateInfo l_frame_buffer_create_info = {};
		l_frame_buffer_create_info.pAttachments = m_attachment.data();
		l_frame_buffer_create_info.height = p_height;
		l_frame_buffer_create_info.attachmentCount = m_attachment.size();
		l_frame_buffer_create_info.renderPass = p_render_pass;
		l_frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		l_frame_buffer_create_info.width = p_width;
		l_frame_buffer_create_info.layers = 1;
		vkCreateFramebuffer(VulkanCore::vk_device, &l_frame_buffer_create_info, nullptr, &m_framebuffer);
	}

	__forceinline void AddAttachment(const VkImageView& p_image) {
		m_attachment.push_back(p_image);
	}


	operator VkFramebuffer() {
		return m_framebuffer;
	}

private:
	VkFramebuffer m_framebuffer;
	std::vector<VkImageView> m_attachment;
};

