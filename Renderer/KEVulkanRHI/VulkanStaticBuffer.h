#pragma once
#include "VulkanResource.h"



//Static Buffer uses a staging buffer to upload data to vram


class KEVulkanStaticBuffer:public KEVulkanResource
{
public:

	KEVulkanStaticBuffer(const uint64_t& p_size,  uint8_t* p_data = nullptr):
	KEVulkanResource(p_size,p_data)
	{
	
	};

	
	~KEVulkanStaticBuffer() {
	
	};

private:

};

