#include "ObjectCubeVK.h"

namespace Vulkan
{
	void ObjectCubeVK::createCube(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, VkRenderPass renderPass, uint32_t images, VkSampleCountFlagBits samples)
	{
		vertices = {
			{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
			{{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},
			{{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}},

			{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},

			{{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
			{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
			{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}},
			{{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}},
			{{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}},

			{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}},

			{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
			{{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}},
			{{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}},
			{{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}},
			{{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}},

			{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
			{{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}}
		};

		createVertexBuffer(device, allocator, commandPool, queue);
		createUniformBuffers(device, allocator, images);
		createDescriptors(device, images);
		createGraphicsPipeline(device, renderPass, samples);
	}

	void ObjectCubeVK::createVertexBuffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue)
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

		vertexCount = (uint32_t)vertices.size();
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

	void ObjectCubeVK::createUniformBuffers(VkDevice device, VmaAllocator allocator, uint32_t images)
	{
		vertUniformBuffers.resize(images);
		vertUniformAllocations.resize(images);

		fragUniformBuffers.resize(images);
		fragUniformAllocations.resize(images);

		VkDeviceSize vertBufferSize = sizeof(VertUniformData);
		VkDeviceSize fragBufferSize = sizeof(FragUniformData);

		VkBufferCreateInfo vertUniformBufferInfo = {};
		vertUniformBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertUniformBufferInfo.size = vertBufferSize;
		vertUniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		vertUniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkBufferCreateInfo fragUniformBufferInfo = {};
		fragUniformBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		fragUniformBufferInfo.size = fragBufferSize;
		fragUniformBufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		fragUniformBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo uniformAllocInfo = {};
		uniformAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		for (size_t i = 0; i < images; i++)
		{
			vmaCreateBuffer(
				allocator,
				&vertUniformBufferInfo,
				&uniformAllocInfo,
				&vertUniformBuffers[i],
				&vertUniformAllocations[i],
				nullptr);

			vmaCreateBuffer(
				allocator,
				&fragUniformBufferInfo,
				&uniformAllocInfo,
				&fragUniformBuffers[i],
				&fragUniformAllocations[i],
				nullptr);
		}
	}

	void ObjectCubeVK::createDescriptors(VkDevice device, uint32_t images)
	{
		VkDescriptorSetLayoutBinding vertLayoutBinding = {};
		vertLayoutBinding.binding = 0;
		vertLayoutBinding.descriptorCount = 1;
		vertLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vertLayoutBinding.pImmutableSamplers = nullptr;
		vertLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutBinding fragLayoutBinding = {};
		fragLayoutBinding.binding = 1;
		fragLayoutBinding.descriptorCount = 1;
		fragLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		fragLayoutBinding.pImmutableSamplers = nullptr;
		fragLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { vertLayoutBinding, fragLayoutBinding };

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = (uint32_t)bindings.size();
		layoutInfo.pBindings = bindings.data();

		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);

		std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = images;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = images;

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
			VkDescriptorBufferInfo vertBufferInfo = {};
			vertBufferInfo.buffer = vertUniformBuffers[i];
			vertBufferInfo.offset = 0;
			vertBufferInfo.range = sizeof(VertUniformData);
			
			VkDescriptorBufferInfo fragBufferInfo = {};
			fragBufferInfo.buffer = fragUniformBuffers[i];
			fragBufferInfo.offset = 0;
			fragBufferInfo.range = sizeof(FragUniformData);

			std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &vertBufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &fragBufferInfo;

			vkUpdateDescriptorSets(
				device,
				(uint32_t)descriptorWrites.size(),
				descriptorWrites.data(),	
				0,
				nullptr);
		}
	}

	void ObjectCubeVK::createGraphicsPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits samples)
	{
		VkShaderModule vertShaderModule = helperVK.createShaderModule(vertFileName, device);
		VkShaderModule fragShaderModule = helperVK.createShaderModule(fragFileName, device);

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
		attributeDescriptions[1].offset = offsetof(Vertex3D, normal);

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
		multiSampling.sampleShadingEnable = VK_TRUE;
		multiSampling.minSampleShading = .2f;
		multiSampling.rasterizationSamples = samples;

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

	void ObjectCubeVK::updateCube(VmaAllocator allocator, uint32_t images, glm::mat4 viewMatrix, glm::vec4 viewVec, int width, int height)
	{
		vertUniformData.view = glm::mat4(1.0f);
		vertUniformData.view = viewMatrix;
		vertUniformData.projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		vertUniformData.projection[1][1] *= -1;
		vertUniformData.model = glm::inverseTranspose(vertUniformData.model);

		fragUniformData.viewPos = viewVec;

		for (size_t i = 0; i < images; i++)
		{
			void* data;
			vmaMapMemory(allocator, vertUniformAllocations[i], &data);
			memcpy(data, &vertUniformData, sizeof(VertUniformData));
			vmaUnmapMemory(allocator, vertUniformAllocations[i]);

			vmaMapMemory(allocator, fragUniformAllocations[i], &data);
			memcpy(data, &fragUniformData, sizeof(FragUniformData));
			vmaUnmapMemory(allocator, fragUniformAllocations[i]);
		}
	}

	void ObjectCubeVK::recordCube(VkDevice device, VkCommandBuffer commandBuffer, uint32_t image, int width, int height)
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
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void ObjectCubeVK::cleanUpCube(VkDevice device, VmaAllocator allocator, uint32_t images)
	{
		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

		for (size_t i = 0; i < images; i++)
		{
			vmaDestroyBuffer(allocator, vertUniformBuffers[i], vertUniformAllocations[i]);
			vmaDestroyBuffer(allocator, fragUniformBuffers[i], fragUniformAllocations[i]);
		}

		vmaDestroyBuffer(allocator, vertexBuffer, vertexAllocation);
	}

	void ObjectCubeVK::reloadPipeline(VkDevice device, VkRenderPass renderPass, VkSampleCountFlagBits samples)
	{
		if (enableWireframe)
		{
			enableWireframe = false;
		}
		
		else if (!enableWireframe)
		{
			enableWireframe = true;
		}

		vkDestroyPipeline(device, graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		createGraphicsPipeline(device, renderPass, samples);
	}
}