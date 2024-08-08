#ifndef SQUARECONSTANTVK_H
#define SQUARECONSTANTVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "HelperVK.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>

namespace Vulkan
{
	class SquareConstantVK
	{
		struct Vertex2D
		{
			glm::vec2 position;
			glm::vec3 color;
		};

		struct PushConstBlock
		{
			glm::mat4 translate;
		} pushConstBlock;

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

		void createSquare(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass);
		void cleanUpSquare(VkDevice device, VmaAllocator allocator);
		void recordSquare(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
		void reloadPipeline(VkDevice device, VkRenderPass renderPass);
	};
}
#endif