#ifndef NUKLEARVK_H
#define NUKLEARVK_H

#include <Windows.h>

#include <NKGUI/nuklear.h>

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
	class NuklearVK
	{
		struct NKDrawVert
		{
			float pos[2];
			float uv[2];
			nk_byte col[4];
		};

		struct PushConstBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

	private:
		VkBuffer vertexBuffer;
		VmaAllocation vertexAllocation;
		VkBuffer indexBuffer;
		VmaAllocation indexAllocation;
		VkImage fontBuffer;
		VmaAllocation fontAllocation;
		VkImageView fontImageView;
		VkSampler fontSampler;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorPool descriptorPool;
		std::vector<VkDescriptorSet> descriptorSets;
		VkPipelineLayout pipelineLayout;
		VkPipeline graphicsPipeline;
		HelperVK helperVK;

		void createFont(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue);
		void createDescriptors(VkDevice device, uint32_t images);
		void createGraphicsPipeline(VkDevice device, VkRenderPass renderPass);
		void createVertexBuffer(VmaAllocator allocator, VkDeviceSize bufferSize);
		void destroyVertexBuffer(VmaAllocator allocator);
		void createIndexBuffer(VmaAllocator allocator, VkDeviceSize bufferSize);
		void destroyIndexBuffer(VmaAllocator allocator);

	public:
		void createNGui(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images, nk_context& nContext);
		void cleanUpNGui(VkDevice device, VmaAllocator allocator, nk_context& nContext);
		void nGuiFrame(GLFWwindow* window, nk_context& nContext);
		void updateNGui(VmaAllocator allocator, nk_context& nContext);
		void recordNGui(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height, nk_context& nContext);
	};
}
#endif