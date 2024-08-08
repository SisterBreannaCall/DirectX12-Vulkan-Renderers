#ifndef LINEVK_H
#define LINEVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include <VMA/vk_mem_alloc.h>

#include "HelperVK.h"

#include <vector>
#include <array>

namespace Vulkan
{
	class LineVK
	{
		struct Vertex2D
		{
			glm::vec2 position;
			glm::vec3 color;
		};

	private:
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		uint32_t vertexCount;
		HelperVK helperVK;

		void createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);

	public:
		std::vector<Vertex2D> vertices;

		void createLine(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass);
		void cleanUpLine(VkDevice device, VmaAllocator allocator);
		void recordLine(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
	};
}
#endif