#ifndef CIRCLEVK_H
#define CIRCLEVK_H

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
	class CircleVK
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
		float xOrigin = 0;
		float yOrigin = 0;
		float xPos = 0;
		float yPos = 0;
		float radius = 1.0f;
		int numberSides = 360;
		bool enableWireframe = false;
		std::vector<Vertex2D> vertices;
		std::vector<uint16_t> indices;
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		VkBuffer indexBuffer;
		VmaAllocation indexAllocation;
		uint32_t indexCount;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		HelperVK helperVK;

		void createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createIndexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);

	public:
		void createCircle(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass);
		void cleanUpCircle(VkDevice device, VmaAllocator allocator);
		void recordCircle(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
		void reloadPipeline(VkDevice device, VkRenderPass renderPass);
	};
}
#endif