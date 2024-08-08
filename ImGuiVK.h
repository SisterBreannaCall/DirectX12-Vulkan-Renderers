#ifndef IMGUIVK_H
#define IMGUIVK_H

#include <Windows.h>

#include <IMGUI/imgui.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "HelperVK.h"

#include <glm/glm.hpp>

#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>
#include <algorithm>

namespace Vulkan
{
	class ImGuiVK
	{
		struct PushConstBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

	private:
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		int32_t vertexCount = 0;
		VkBuffer indexBuffer;
		VmaAllocation indexAllocation;
		int32_t indexCount;
		VkImage fontBuffer;
		VmaAllocation fontAllocation;
		VkImageView fontImageView;
		VkSampler fontSampler;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkViewport viewport;
		VkPipelineCache pipelineCache;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		HelperVK helperVK;

		void createFont(VmaAllocator allocator, VkDevice device, VkCommandPool commandPool, VkQueue queue);
		void createDescriptors(VkDevice device, uint32_t images);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);
		void createVertexBuffer(VmaAllocator allocator, VkDeviceSize bufferSize);
		void destroyVertexBuffer(VmaAllocator allocator);
		void createIndexBuffer(VmaAllocator allocator, VkDeviceSize bufferSize);
		void destroyIndexBuffer(VmaAllocator allocator);

	public:
		void createIGui(VmaAllocator allocator, VkDevice device, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images, int width, int height);
		void cleanUpIGui(VkDevice device, VmaAllocator allocator);
		void updateIGui(VmaAllocator allocator);
		void recordIGui(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height);
		void iGuiFrame(GLFWwindow* window);
	};
}
#endif