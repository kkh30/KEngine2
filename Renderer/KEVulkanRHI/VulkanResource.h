#pragma once
#include <stdint.h>
#include <memory>
#include "VulkanCore.h"
class KEVulkanResource
{
public:
	KEVulkanResource(const uint64_t& p_size = 0, uint8_t* p_data=nullptr) :
		m_size(p_size),
		m_data(p_data)
	{

	};

	virtual ~KEVulkanResource() {
		if (m_data) {
			delete[] m_data;
			m_data = nullptr;
		}
	};

	virtual void Write(void* p_src, uint64_t p_size) {
		memmove(m_data, p_src, static_cast<uint64_t>(p_size));
	}

	virtual void Read() {
	
	}

	KEVulkanResource(const KEVulkanResource&) = delete;
	KEVulkanResource& operator =(const KEVulkanResource&) = delete;

protected:
	uint64_t m_size;
	uint8_t* m_data;
};

