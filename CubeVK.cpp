#include "CubeVK.h"

namespace Vulkan
{
	void CubeVK::createCube(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images)
	{
		vertices = {
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},

			{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
			{{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},

			{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},

			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},

			{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
			{{-0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}},
			{{-0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}},

			{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 0.0f}}
		};

		indices = {
			0, 1, 2, 3, 4, 5,
			6, 7, 8, 9, 10, 11,
			12, 13, 14, 15, 16, 17,
			18, 19, 20, 21, 22, 23,
		    24, 25, 26, 27, 28, 29,
			30, 31, 32, 33, 34, 35
		};

		createVertexBuffer(device, allocator, commandPool, queue);
		createIndexBuffer(device, allocator, commandPool, queue);
		createUniformBuffers(device, allocator, images);
		createDescriptors(device, images);
		createGraphicsPipeline(device, renderPass);
	}

	void CubeVK::createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.size = bufferSize;
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
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vmaUnmapMemory(allocator, stagingAllocation);

		vertices.clear();
		vertices.shrink_to_fit();

		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.size = bufferSize;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		vertexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo vertexAllocInfo = {};
		vertexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateBuffer(
			allocator,
			&vertexBufferInfo,
			&vertexAllocInfo,
			&vertexBuffer,
			&vertexAllocation, 
			nullptr);

		helperVK.copyBuffer(
			stagingBuffer,
			vertexBuffer,
			bufferSize,
			device, 
			commandPool,
			queue);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	}

	void CubeVK::createIndexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
	{
		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBufferCreateInfo stagingBufferInfo = {};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		stagingBufferInfo.size = bufferSize;
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
		memcpy(data, indices.data(), (size_t)bufferSize);
		vmaUnmapMemory(allocator, stagingAllocation);

		indexCount = (uint32_t)indices.size();
		indices.clear();
		indices.shrink_to_fit();

		VkBufferCreateInfo indexBufferInfo = {};
		indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		indexBufferInfo.size = bufferSize;
		indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		indexBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo indexAllocInfo = {};
		indexAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vmaCreateBuffer(
			allocator, 
			&indexBufferInfo,
			&indexAllocInfo,
			&indexBuffer,
			&indexAllocation,
			nullptr);

		helperVK.copyBuffer(
			stagingBuffer,
			indexBuffer,
			bufferSize,
			device,
			commandPool,
			queue);

		vmaDestroyBuffer(allocator, stagingBuffer, stagingAllocation);
	}

	void CubeVK::createUniformBuffers(VkDevice device, VmaAllocator allocator, uint32_t images)
	{
		VkDeviceSize bufferSize = sizeof(UniformData);
		uniformBuffers.resize(images);
		uniformAllocations.resize(images);

		for (size_t i = 0; i < images; i++)
		{
			VkBufferCreateInfo uniformBufferInfo = {};
			uniformBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			uniformBufferInfo.size = bufferSize;
			uniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			uniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VmaAllocationCreateInfo uniformAllocInfo = {};
			uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

			vmaCreateBuffer(
				allocator,
				&uniformBufferInfo,
				&uniformAllocInfo,
				&uniformBuffers[i],
				&uniformAllocations[i],
				nullptr);
		}
	}

	void CubeVK::createDescriptors(VkDevice device, uint32_t images)
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)bindings.size();
		layoutInfo.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

		std::array<VkDescriptorPoolSize, 1> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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

		for (uint32_t i = 0; i < images; i++)
		{
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformData);

			std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(
				device, 
				(uint32_t)descriptorWrites.size(), 
				descriptorWrites.data(),
				0, 
				nullptr);
		}
	}

	void CubeVK::createGraphicsPipeline(VkDevice device, VkRenderPass renderPass)
	{
		VkShaderModule vertShaderModule = helperVK.createShaderModule("data/shadersvk/VKCUBEV.spv", device);
		VkShaderModule fragShaderModule = helperVK.createShaderModule("data/shadersvk/VK2DF.spv", device);

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
		bindingDescription.stride = sizeof(Vertex3D);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex3D, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex3D, color);

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

		if (enableWireframe)
			rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
		if (!enableWireframe)
			rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_NONE;
		rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multiSampling = {};
		multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSampling.sampleShadingEnable = VK_FALSE;
		multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

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
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.pDepthStencilState = &depthStencil;
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

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
	}

	void CubeVK::updateCube(VmaAllocator allocator, uint32_t images, glm::mat4 viewMatrix, int width, int height)
	{
		for (size_t i = 0; i < images; i++)
		{
			uniformData.model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
			uniformData.view = glm::mat4(1.0f);
			uniformData.view = viewMatrix;
			uniformData.projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
			uniformData.projection[1][1] *= -1;

			void* data;
			vmaMapMemory(allocator, uniformAllocations[i], &data);
			memcpy(data, &uniformData, sizeof(UniformData));
			vmaUnmapMemory(allocator, uniformAllocations[i]);
		}
	}

	void CubeVK::recordCube(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height)
	{
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

		VkRect2D scissorRect;
		scissorRect.offset.x = 0;
		scissorRect.offset.y = 0;
		scissorRect.extent.width = width;
		scissorRect.extent.height = height;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);

		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0,
			1,
			&descriptorSets[image],
			0,
			nullptr);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}

	void CubeVK::cleanUpCube(VkDevice device, VmaAllocator allocator, uint32_t images)
	{
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		for (size_t i = 0; i < images; i++)
		{
			vmaDestroyBuffer(allocator, uniformBuffers[i], uniformAllocations[i]);
		}

		vmaDestroyBuffer(allocator, indexBuffer, indexAllocation);
		vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
	}

	void CubeVK::reloadPipeline(VkDevice device, VkRenderPass renderPass)
	{
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		createGraphicsPipeline(device, renderPass);
	}
}