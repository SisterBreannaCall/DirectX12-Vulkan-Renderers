#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_IMPLEMENTATION

#include "NuklearVK.h"

static nk_font_atlas nAtlas;
static nk_font* nFont;
static enum nk_anti_aliasing AA;
static nk_buffer cmds;

namespace Vulkan
{
	void NuklearVK::createNGui(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images, nk_context& nContext)
	{
		struct nk_font_config cfg = nk_font_config(0);
		cfg.oversample_h = 3;
		cfg.oversample_v = 2;

		nk_font_atlas_init_default(&nAtlas);
		nk_font_atlas_begin(&nAtlas);
		nFont = nk_font_atlas_add_default(&nAtlas, 14.0f, &cfg);

		createFont(device, allocator, commandPool, queue);
		createDescriptors(device, images);
		createGraphicsPipeline(device, renderPass);

		nk_font_atlas_end(&nAtlas, nk_handle_ptr(fontImageView), nullptr);
		nk_init_default(&nContext, &nFont->handle);
		nk_buffer_init_default(&cmds);
	}

	void NuklearVK::createFont(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
	{
		const void* image;
		int texWidth, texHeight;
		image = nk_font_atlas_bake(&nAtlas, &texWidth, &texHeight, NK_FONT_ATLAS_RGBA32);

		VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(char);

		VkBufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.size = imageSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo stagingAllocInfo = {};
		stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;

		vmaCreateBuffer(
			allocator,
			&stagingBufferInfo,
			&stagingAllocInfo,
			&stagingBuffer,
			&stagingAllocation,
			nullptr);

		void* data;
		vmaMapMemory(allocator, stagingAllocation, &data);
		memcpy(data, image, (size_t)imageSize);
		vmaUnmapMemory(allocator, stagingAllocation);

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

		VmaAllocationCreateInfo imageAllocInfo = {};
		imageAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateImage(
			allocator,
			&imageInfo,
			&imageAllocInfo,
			&fontBuffer,
			&fontAllocation,
			nullptr);

		helperVK.transitionImageLayout(
			fontBuffer,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			device,
			commandPool,
			queue);

		helperVK.copyBufferToImage(
			stagingBuffer,
			fontBuffer,
			texWidth,
			texHeight,
			device,
			commandPool,
			queue);

		helperVK.transitionImageLayout(
			fontBuffer,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			device,
			commandPool,
			queue);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);

		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = fontBuffer;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = imageInfo.format;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		vkCreateImageView(device, &viewInfo, nullptr, &fontImageView);

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = VK_FALSE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;

		vkCreateSampler(device, &samplerInfo, nullptr, &fontSampler);
	}

	void NuklearVK::createDescriptors(VkDevice device, uint32_t images)
	{
		VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.pImmutableSamplers = nullptr;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)bindings.size();
		layoutInfo.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

		std::array<VkDescriptorPoolSize, 1> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = images;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = images;

		vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

		std::vector<VkDescriptorSetLayout> layouts(images, descriptorSetLayout);
		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = images;
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(images);
		vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());

		for (size_t i = 0; i < images; i++)
		{
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = fontImageView;
			imageInfo.sampler = fontSampler;

			std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 1;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(
				device,
				(uint32_t)descriptorWrites.size(),
				descriptorWrites.data(),
				0,
				nullptr);
		}
	}

	void NuklearVK::createGraphicsPipeline(VkDevice device, VkRenderPass renderPass)
	{
		VkShaderModule vertShaderModule = helperVK.createShaderModule("data/shadersvk/UIV.spv", device);
		VkShaderModule fragShaderModule = helperVK.createShaderModule("data/shadersvk/UIF.spv", device);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(NKDrawVert);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(NKDrawVert, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(NKDrawVert, uv);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R8G8B8A8_UNORM;
		attributeDescriptions[2].offset = offsetof(NKDrawVert, col);

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.scissorCount = 1;
		viewportState.flags = 0;

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates = dynamicStateEnables.data();
		dynamicState.dynamicStateCount = (uint32_t)dynamicStateEnables.size();
		dynamicState.flags = 0;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multiSampling = {};
		multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSampling.sampleShadingEnable = VK_FALSE;
		multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f;
		colorBlending.blendConstants[1] = 0.0f;
		colorBlending.blendConstants[2] = 0.0f;
		colorBlending.blendConstants[3] = 0.0f;

		VkPushConstantRange pushConstantRange = {};
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstBlock);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multiSampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = renderPass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		vkCreateGraphicsPipelines(
			device,
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&graphicsPipeline);

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	void NuklearVK::createVertexBuffer(VmaAllocator allocator, VkDeviceSize bufferSize)
	{
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.size = bufferSize;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vertexAllocInfo = {};
		vertexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		vmaCreateBuffer(
			allocator,
			&vertexBufferInfo,
			&vertexAllocInfo,
			&vertexBuffer,
			&vertexAllocation,
			nullptr);
	}

	void NuklearVK::destroyVertexBuffer(VmaAllocator allocator)
	{
		vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
	}

	void NuklearVK::createIndexBuffer(VmaAllocator allocator, VkDeviceSize bufferSize)
	{
		VkBufferCreateInfo indexBufferInfo = {};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.size = bufferSize;
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo indexAllocInfo = {};
		indexAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		vmaCreateBuffer(
			allocator,
			&indexBufferInfo,
			&indexAllocInfo,
			&indexBuffer,
			&indexAllocation,
			nullptr);
	}

	void NuklearVK::destroyIndexBuffer(VmaAllocator allocator)
	{
		vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
	}

	void NuklearVK::nGuiFrame(GLFWwindow* window, nk_context& nContext)
	{
		nk_clear(&nContext);
		nk_buffer_clear(&cmds);
		nk_style_set_font(&nContext, &nFont->handle);

		nk_input_begin(&nContext);
		double mouse_x;
		double mouse_y;
		glfwGetCursorPos(window, &mouse_x, &mouse_y);

		if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
		{
			nk_input_motion(&nContext, (int)mouse_x, (int)mouse_y);
		}
		else
		{
			nk_input_motion(&nContext, -1, -1);
		}

		nk_input_button(&nContext, NK_BUTTON_LEFT, (int)mouse_x, (int)mouse_y, glfwGetMouseButton(window, 0) != 0);
		nk_input_button(&nContext, NK_BUTTON_MIDDLE, (int)mouse_x, (int)mouse_y, glfwGetMouseButton(window, 1) != 0);
		nk_input_button(&nContext, NK_BUTTON_RIGHT, (int)mouse_x, (int)mouse_y, glfwGetMouseButton(window, 2) != 0);
		nk_input_end(&nContext);
	}

	void NuklearVK::updateNGui(VmaAllocator allocator, nk_context& nContext)
	{
		nk_convert_config config;
		static const nk_draw_vertex_layout_element vertex_layout[] = {
			{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(NKDrawVert, pos)},
			{NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(NKDrawVert, uv)},
			{NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(NKDrawVert, col)},
			{NK_VERTEX_LAYOUT_END}
		};

		config.vertex_layout = vertex_layout;
		config.vertex_size = sizeof(NKDrawVert);
		config.vertex_alignment = NK_ALIGNOF(NKDrawVert);
		config.circle_segment_count = 22;
		config.curve_segment_count = 22;
		config.arc_segment_count = 22;
		config.global_alpha = 1.0f;
		config.shape_AA = AA;
		config.line_AA = AA;

		VkDeviceSize vertexBufferSize = 512 * 1024;
		VkDeviceSize indexBufferSize = 128 * 1024;

		if (vertexBuffer == VK_NULL_HANDLE)
		{
			createVertexBuffer(allocator, vertexBufferSize);
		}

		if (indexBuffer == VK_NULL_HANDLE)
		{
			createIndexBuffer(allocator, indexBufferSize);
		}

		nk_buffer vBuffer;
		nk_buffer iBuffer;

		void* vtxResouce;
		void* idxResouce;

		vmaMapMemory(allocator, vertexAllocation, &vtxResouce);
		vmaMapMemory(allocator, indexAllocation, &idxResouce);

		nk_buffer_init_fixed(&vBuffer, vtxResouce, vertexBufferSize);
		nk_buffer_init_fixed(&iBuffer, idxResouce, indexBufferSize);

		nk_convert(&nContext, &cmds, &vBuffer, &iBuffer, &config);

		vmaFlushAllocation(allocator, vertexAllocation, 0, vertexBufferSize);
		vmaFlushAllocation(allocator, indexAllocation, 0, indexBufferSize);

		vmaUnmapMemory(allocator, vertexAllocation);
		vmaUnmapMemory(allocator, indexAllocation);

		nk_buffer_free(&vBuffer);
		nk_buffer_free(&iBuffer);
	}

	void NuklearVK::recordNGui(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height, nk_context& nContext)
	{
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSets[image],
			0,
			nullptr);

		vkCmdBindPipeline(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			graphicsPipeline);

		VkViewport viewport = {};
		viewport.x = 0;
		viewport.y = 0;
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		pushConstBlock.scale = glm::vec2(2.0f / width, 2.0f / height);
		pushConstBlock.translate = glm::vec2(-1.0f);
		vkCmdPushConstants(
			commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(PushConstBlock),
			&pushConstBlock);

		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if (cmds.needed > 0)
		{
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

			const nk_draw_command* cmd;
			nk_draw_foreach(cmd, &nContext, &cmds)
			{
				if (!cmd->elem_count)
					continue;

				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(cmd->clip_rect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(cmd->clip_rect.y), 0);
				scissorRect.extent.width = (uint32_t)(cmd->clip_rect.w);
				scissorRect.extent.height = (uint32_t)(cmd->clip_rect.h);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(commandBuffer, cmd->elem_count, 1, indexOffset, vertexOffset, 0);
				indexOffset += cmd->elem_count;
			}
		}
	}

	void NuklearVK::cleanUpNGui(VkDevice device, VmaAllocator allocator, nk_context& nContext)
	{
		destroyIndexBuffer(allocator);
		destroyVertexBuffer(allocator);
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
		vkDestroySampler(device, fontSampler, nullptr);
		vkDestroyImageView(device, fontImageView, nullptr);
		vmaDestroyImage(allocator, fontBuffer, fontAllocation);

		nk_buffer_free(&cmds);
		nk_font_atlas_clear(&nAtlas);
		nk_free(&nContext);
	}
}