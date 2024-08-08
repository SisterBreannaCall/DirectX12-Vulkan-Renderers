#ifndef SQUARETEXVK_H
#define SQUARETEXVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "HelperVK.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VMA/vk_mem_alloc.h>

#include <STB/stb_image.h>

#include <vector>
#include <array>

namespace Vulkan
{
	class SquareTexVK
	{
		struct Vertex2D
		{
			glm::vec2 position;
			glm::vec3 color;
			glm::vec2 texCoord;
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
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);

	public:
		std::vector<Vertex2D> vertices;
		std::vector<uint16_t> indices;
		std::string textureFileName;
		bool enableWireframe = false;

		void createSquare(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images);
		void cleanUpSquare(VkDevice device, VmaAllocator allocator, uint32_t images);
		void recordSquare(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
		void reloadPipeline(VkDevice device, VkRenderPass renderPass);
	};
}
#endif