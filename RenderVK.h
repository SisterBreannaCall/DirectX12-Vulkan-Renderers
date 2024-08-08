#ifndef RENDERVK_H
#define RENDERVK_H

#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include "HelperVK.h"

#include <glm/glm.hpp>
#include <VMA/vk_mem_alloc.h>

#include <vector>
#include <array>
#include <iostream>

namespace Vulkan
{
	class RenderVK
	{
	private:
		void createInstance();
		void setupDebugMessenger();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createSurface(GLFWwindow* window);
		void createSwapChain();
		void createImageViews();
		void createRenderPass();
		void createFramebuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createVMAAllocator();
		void createColorResources();
		void createDepthResources();
		void createSyncObjects();
		void cleanUpSwapChain();

	public:
		int width;
		int height;
		bool enableDebug;
		std::array<VkClearValue, 2> clearValues;
		uint32_t engineVersion;
		uint32_t appVersion;
		HelperVK helperVK;
		VkInstance instance;
		VkDebugUtilsMessengerEXT debugMessenger;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		VkSurfaceKHR surface;
		VkDevice device;
		VkQueue presentQueue;
		VkSwapchainKHR swapChain;
		VkFormat swapChainImageFormat;
		VmaAllocator allocator;
		VkQueue graphicsQueue;
		VkRenderPass renderPass;
		VkCommandPool commandPool;
		VkExtent2D swapChainExtent;
		VkImage depthImage;
		VmaAllocation depthAllocation;
		VkImageView depthImageView;
		VkImage colorImage;
		VmaAllocation colorImageAllocation;
		VkImageView colorImageView;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		std::vector<VkFramebuffer> swapChainFramebuffers;
		std::vector<VkCommandBuffer> commandBuffers;
		size_t currentFrame = 0;
		const int maxFramesInFlight = 2;
		uint32_t numImages;

		void initVK(GLFWwindow* window);
		void drawFrame();
		void flushDevice();
		void beginRecord();
		void endRecord();
		void cleanUpVK();
		void recreateSwapChain(int vWidth, int vHeight);
		void recreate();
	};
}
#endif