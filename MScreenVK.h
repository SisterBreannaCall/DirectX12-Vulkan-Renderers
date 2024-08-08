#ifndef MSCREENVK_H
#define MSCREENVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "HelperVK.h"

#include <glm/glm.hpp>

#include <VMA/vk_mem_alloc.h>

#include "MFont.h"

#include <vector>
#include <array>

namespace Vulkan
{
	class MScreenVK
	{
		struct Vertex2D
		{
			glm::vec2 position;
			glm::vec3 color;
			glm::vec2 texCoord;
		};

		struct falling_string
		{
			int32_t x = 0;
			float y = 0.0f;
			float speed = 0.0f;
			int32_t length = 0;
		};

	private:
		const int32_t s_col_count = 80;
		const int32_t s_falling_strings_count = 1500;
		const int32_t s_falling_string_min_length = 15;
		const int32_t s_falling_string_max_length = 40;
		const int32_t s_falling_string_min_speed = 10;
		const int32_t s_falling_string_max_speed = 30;

		std::vector<Vertex2D> vertices;
		std::vector<uint16_t> indices;
		MScreen::MFont mFont;
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		VkBuffer indexBuffer;
		VmaAllocation indexAllocation;
		uint32_t indexCount = 0;
		VkImage imageBuffer;
		VmaAllocation imageAllocation;
		VkImageView imageView;
		VkSampler imageSampler;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		HelperVK helperVK;
		
		void createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createIndexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createTexture(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createDescriptors(VkDevice device, uint32_t images);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits samples);

	public:
		void createMScreen(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images, VkSampleCountFlagBits samples);
		void cleanUpMScreen(VkDevice device, VmaAllocator allocator, uint32_t images);
		void recordMScreen(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
	};
}
#endif