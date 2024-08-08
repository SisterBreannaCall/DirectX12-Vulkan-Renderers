#ifndef TRIANGLEVK_H
#define TRIANGLEVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "HelperVK.h"

#include <glm/glm.hpp>

#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>

namespace Vulkan
{
	class TriangleVK
	{
		struct Vertex2D
		{
			glm::vec2 position;
			glm::vec3 color;
		};

	private:
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		VkBuffer indexBuffer;
		VmaAllocation indexAllocation;
		uint32_t indexCount = 0;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		HelperVK helperVK;

		void createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createIndexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);
		
	public:
		std::vector<Vertex2D> vertices;
		std::vector<uint16_t> indices;
		bool enableWireframe = false;

		void createTriangle(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass);
		void cleanUpTriangle(VkDevice device, VmaAllocator allocator);
		void recordTriangle(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
		void reloadPipeline(VkDevice device, VkRenderPass renderPass);
	};
}
#endif