#pragma once
#include "VulkanResource.h"


//Dynamic buffer is used when data is coherent between cpu and gpu such as uniform buffers.
class KEVulkanDynamicBuffer :public KEVulkanResource
{
public:
	KEVulkanDynamicBuffer(const uint64_t& p_size, VmaAllocator* p_allocator, uint8_t* p_data = nullptr):
	KEVulkanResource(p_size, p_allocator, p_data)
	{
	
	};
	~KEVulkanDynamicBuffer() {
	
	};

private:

};

