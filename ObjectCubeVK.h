#ifndef OBJECTCUBEVK_H
#define OBJECTCUBEVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "HelperVK.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>

namespace Vulkan
{
	class ObjectCubeVK
	{
		struct Vertex3D
		{
			glm::vec3 position;
			glm::vec3 normal;
		};

		struct VertUniformData
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
		};

		struct FragUniformData
		{
			glm::vec4 lightColor;
			glm::vec4 objectColor;
			glm::vec4 lightPos;
			glm::vec4 viewPos;
		};

	private:
		std::vector<Vertex3D> vertices;
		bool enableWireframe = false;
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		uint32_t vertexCount = 0;
		std::vector<VkBuffer> vertUniformBuffers;
		std::vector<VmaAllocation> vertUniformAllocations;
		std::vector<VkBuffer> fragUniformBuffers;
		std::vector<VmaAllocation> fragUniformAllocations;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		HelperVK helperVK;

		void createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createUniformBuffers(VkDevice device, VmaAllocator allocator, uint32_t images);
		void createDescriptors(VkDevice device, uint32_t images);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits samples);

	public:
		std::string vertFileName;
		std::string fragFileName;
		VertUniformData vertUniformData;
		FragUniformData fragUniformData;

		void createCube(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images, VkSampleCountFlagBits samples);
		void updateCube(VmaAllocator allocator, uint32_t images, glm::mat4 viewMatrix, glm::vec4 viewVec, int width, int height);
		void recordCube(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
		void cleanUpCube(VkDevice device, VmaAllocator allocator, uint32_t images);
		void reloadPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits samples);
	};
}
#endif