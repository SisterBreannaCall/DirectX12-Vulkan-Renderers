#ifndef CUBEVK_H
#define CUBEVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "HelperVK.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>

namespace Vulkan
{
	class CubeVK
	{
		struct Vertex3D
		{
			glm::vec3 position;
			glm::vec3 color;
		};

		struct UniformData
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
		} uniformData;

	private:
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		VkBuffer indexBuffer;
		VmaAllocation indexAllocation;
		uint32_t indexCount = 0;
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VmaAllocation> uniformAllocations;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		HelperVK helperVK;

		void createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createIndexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createUniformBuffers(VkDevice device, VmaAllocator allocator, uint32_t images);
		void createDescriptors(VkDevice device, uint32_t images);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);

	public:
		std::vector<Vertex3D> vertices;
		std::vector<uint16_t> indices;
		bool enableWireframe = false;

		void createCube(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images);
		void cleanUpCube(VkDevice device, VmaAllocator allocator, uint32_t images);
		void updateCube(VmaAllocator allocator, uint32_t images, glm::mat4 viewMatrix, int width, int height);
		void recordCube(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
		void reloadPipeline(VkDevice device, VkRenderPass renderPass);
	};
}
#endif