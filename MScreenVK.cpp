#include "MScreenVK.h"

namespace Vulkan
{
	void MScreenVK::createMScreen(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images, VkSampleCountFlagBits samples)
	{
		mFont.loadFont();
		

		size_t vertexBufferSize = 

		
	}

	void MScreenVK::createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
	{
		
	}

	void MScreenVK::createIndexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
	{
		
	}

	void MScreenVK::createTexture(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
	{
		
	}

	void MScreenVK::createDescriptors(VkDevice device, uint32_t images)
	{
		
	}

	void MScreenVK::createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits samples)
	{
		
	}

	void MScreenVK::recordMScreen(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height)
	{
		
	}

	void MScreenVK::cleanUpMScreen(VkDevice device, VmaAllocator allocator, uint32_t images)
	{
		
	}
}