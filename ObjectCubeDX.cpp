#include "ObjectCubeDX.h"

namespace DirectX
{
	void ObjectCubeDX::createCube(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue, UINT msaaSamples)
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

		createVertexBuffer(device, allocator, queue);
		createDescriptorHeap(device);
		createUniformBuffers(device, allocator);
		createGraphicsPipeline(device, msaaSamples);
	}

	void ObjectCubeDX::createVertexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
	{
		size_t bufferSize = sizeof(vertices[0]) * vertices.size();
		auto commandList = queue->getCommandList(device);

		D3D12_RESOURCE_DESC stagingBufferDesc = {};
		stagingBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		stagingBufferDesc.Alignment = 0;
		stagingBufferDesc.Width = bufferSize;
		stagingBufferDesc.Height = 1;
		stagingBufferDesc.DepthOrArraySize = 1;
		stagingBufferDesc.MipLevels = 1;
		stagingBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		stagingBufferDesc.SampleDesc.Count = 1;
		stagingBufferDesc.SampleDesc.Quality = 0;
		stagingBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		stagingBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC stagingAllocDesc = {};
		stagingAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		ComPtr<ID3D12Resource> stagingBuffer;
		D3D12MA::Allocation* stagingAllocation = nullptr;

		allocator->CreateResource(
			&stagingAllocDesc,
			&stagingBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&stagingAllocation,
			IID_PPV_ARGS(&stagingBuffer));

		void* data;
		CD3DX12_RANGE readRange(0, 0);
		stagingBuffer->Map(0, &readRange, &data);
		memcpy(data, vertices.data(), bufferSize);
		stagingBuffer->Unmap(0, nullptr);

		vertexCount = (UINT)vertices.size();
		vertices.clear();
		vertices.shrink_to_fit();

		D3D12_RESOURCE_DESC vertexBufferDesc = {};
		vertexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vertexBufferDesc.Alignment = 0;
		vertexBufferDesc.Width = bufferSize;
		vertexBufferDesc.Height = 1;
		vertexBufferDesc.DepthOrArraySize = 1;
		vertexBufferDesc.MipLevels = 1;
		vertexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		vertexBufferDesc.SampleDesc.Count = 1;
		vertexBufferDesc.SampleDesc.Quality = 0;
		vertexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		vertexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC vertexAllocDesc = {};
		vertexAllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		allocator->CreateResource(
			&vertexAllocDesc,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			&vertexAllocation,
			IID_PPV_ARGS(&vertexBuffer));

		commandList->CopyResource(vertexBuffer.Get(), stagingBuffer.Get());

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
		vertexBufferView.SizeInBytes = (UINT)bufferSize;
		vertexBufferView.StrideInBytes = sizeof(Vertex3D);

		auto fenceValue = queue->executeCommandList(commandList);
		queue->waitForFenceValue(fenceValue);

		stagingAllocation->Release();
	}

	void ObjectCubeDX::createDescriptorHeap(ComPtr<ID3D12Device2> device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 2;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&uniformDescriptorHeap));

		uniformDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		uniformDescriptorHandle = uniformDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	}

	void ObjectCubeDX::createUniformBuffers(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator)
	{
		size_t vertBufferSize = sizeof(VertUniformData);
		size_t fragBufferSize = sizeof(FragUniformData);

		D3D12_RESOURCE_DESC vertUniformBufferDesc = {};
		vertUniformBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		vertUniformBufferDesc.Alignment = 0;
		vertUniformBufferDesc.Width = vertBufferSize;
		vertUniformBufferDesc.Height = 1;
		vertUniformBufferDesc.DepthOrArraySize = 1;
		vertUniformBufferDesc.MipLevels = 1;
		vertUniformBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		vertUniformBufferDesc.SampleDesc.Count = 1;
		vertUniformBufferDesc.SampleDesc.Quality = 0;
		vertUniformBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		vertUniformBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_RESOURCE_DESC fragUniformBufferDesc = {};
		fragUniformBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		fragUniformBufferDesc.Alignment = 0;
		fragUniformBufferDesc.Width = fragBufferSize;
		fragUniformBufferDesc.Height = 1;
		fragUniformBufferDesc.DepthOrArraySize = 1;
		fragUniformBufferDesc.MipLevels = 1;
		fragUniformBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		fragUniformBufferDesc.SampleDesc.Count = 1;
		fragUniformBufferDesc.SampleDesc.Quality = 0;
		fragUniformBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		fragUniformBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC uniformAllocDesc = {};
		uniformAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		allocator->CreateResource(
			&uniformAllocDesc,
			&vertUniformBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&vertUniformAllocation,
			IID_PPV_ARGS(&vertUniformBuffer));

		D3D12_CONSTANT_BUFFER_VIEW_DESC vertUniformViewDesc = {};
		vertUniformViewDesc.BufferLocation = vertUniformBuffer->GetGPUVirtualAddress();
		vertUniformViewDesc.SizeInBytes = (UINT)vertBufferSize;
		device->CreateConstantBufferView(&vertUniformViewDesc, uniformDescriptorHandle);
		uniformDescriptorHandle.Offset(uniformDescriptorSize);

		allocator->CreateResource(
			&uniformAllocDesc,
			&fragUniformBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&fragUniformAllocation,
			IID_PPV_ARGS(&fragUniformBuffer));

		D3D12_CONSTANT_BUFFER_VIEW_DESC fragUniformViewDesc = {};
		fragUniformViewDesc.BufferLocation = fragUniformBuffer->GetGPUVirtualAddress();
		fragUniformViewDesc.SizeInBytes = (UINT)fragBufferSize;
		device->CreateConstantBufferView(&fragUniformViewDesc, uniformDescriptorHandle);
		uniformDescriptorHandle.Offset(uniformDescriptorSize);
	}

	void ObjectCubeDX::createGraphicsPipeline(ComPtr<ID3D12Device2> device, UINT msaaSamples)
	{
		ComPtr<ID3DBlob> vertShaderBlob;
		ComPtr<ID3DBlob> fragShaderBlob;

		D3DReadFileToBlob(vertFileName, &vertShaderBlob);
		D3DReadFileToBlob(fragFileName, &fragShaderBlob);

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(Vertex3D, position),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},

			{"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(Vertex3D, normal),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0}
		};

		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
		if (FAILED(device->CheckFeatureSupport(
			D3D12_FEATURE_ROOT_SIGNATURE,
			&featureData,
			sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		CD3DX12_RASTERIZER_DESC rasterizer = {};

		if (enableWireframe)
		{
			rasterizer.FillMode = D3D12_FILL_MODE_WIREFRAME;
		}
		if (!enableWireframe)
		{
			rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
		}

		rasterizer.CullMode = D3D12_CULL_MODE_NONE;
		rasterizer.FrontCounterClockwise = FALSE;
		rasterizer.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		rasterizer.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizer.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizer.DepthClipEnable = TRUE;
		rasterizer.MultisampleEnable = FALSE;
		rasterizer.AntialiasedLineEnable = FALSE;
		rasterizer.ForcedSampleCount = 0;
		rasterizer.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		D3D12_DEPTH_STENCIL_DESC depthDesc = {};
		depthDesc.DepthEnable = TRUE;
		depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		depthDesc.StencilEnable = FALSE;

		CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
		ranges[0].Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			1,
			0,
			0,
			D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		ranges[1].Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			1,
			1,
			0,
			D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(
			1,
			&ranges[0],
			D3D12_SHADER_VISIBILITY_VERTEX);
		rootParameters[1].InitAsDescriptorTable(
			1,
			&ranges[1],
			D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			_countof(rootParameters),
			rootParameters,
			0,
			nullptr,
			rootSignatureFlags);

		ComPtr<ID3DBlob> rootSignatureBlob;
		ComPtr<ID3DBlob> errorBlob;

		D3DX12SerializeVersionedRootSignature(
			&rootSignatureDesc,
			featureData.HighestVersion,
			&rootSignatureBlob,
			&errorBlob);

		device->CreateRootSignature(
			0,
			rootSignatureBlob->GetBufferPointer(),
			rootSignatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature));

		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc = {};
		graphicsDesc.InputLayout = { inputLayout, _countof(inputLayout) };
		graphicsDesc.pRootSignature = rootSignature.Get();
		graphicsDesc.VS = CD3DX12_SHADER_BYTECODE(vertShaderBlob.Get());
		graphicsDesc.PS = CD3DX12_SHADER_BYTECODE(fragShaderBlob.Get());
		graphicsDesc.RasterizerState = rasterizer;
		graphicsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		graphicsDesc.DepthStencilState = depthDesc;
		graphicsDesc.SampleMask = UINT_MAX;
		graphicsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		graphicsDesc.NumRenderTargets = 1;
		graphicsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		graphicsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		graphicsDesc.SampleDesc.Count = msaaSamples;

		device->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(&graphicsPipeline));
	}

	void ObjectCubeDX::updateCube(glm::mat4 viewMatrix, glm::vec4 viewVec, int width, int height)
	{
		vertUniformData.view = glm::mat4(1.0f);
		vertUniformData.view = viewMatrix;
		vertUniformData.projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
		vertUniformData.model = glm::inverseTranspose(vertUniformData.model);

		fragUniformData.viewPos = viewVec;

		void* vertData;
		CD3DX12_RANGE vertReadRange(0, 0);
		vertUniformBuffer->Map(0, &vertReadRange, &vertData);
		memcpy(vertData, &vertUniformData, sizeof(VertUniformData));
		vertUniformBuffer->Unmap(0, nullptr);

		void* fragData;
		CD3DX12_RANGE fragReadRange(0, 0);
		fragUniformBuffer->Map(0, &fragReadRange, &fragData);
		memcpy(fragData, &fragUniformData, sizeof(FragUniformData));
		fragUniformBuffer->Unmap(0, nullptr);
	}

	void ObjectCubeDX::recordCube(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv, int width, int height)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHeapStart = uniformDescriptorHeap->GetGPUDescriptorHandleForHeapStart();

		D3D12_VIEWPORT viewport;
		viewport.Width = (float)width;
		viewport.Height = (float)height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;

		commandList->RSSetViewports(1, &viewport);

		CD3DX12_RECT scissorRect(0, 0, LONG_MAX, LONG_MAX);
		commandList->RSSetScissorRects(1, &scissorRect);

		commandList->SetGraphicsRootSignature(rootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[]{ uniformDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		CD3DX12_GPU_DESCRIPTOR_HANDLE vertUHandle(gpuHeapStart, 0, uniformDescriptorSize);
		commandList->SetGraphicsRootDescriptorTable(0, vertUHandle);

		CD3DX12_GPU_DESCRIPTOR_HANDLE fragUHandle(gpuHeapStart, 1, uniformDescriptorSize);
		commandList->SetGraphicsRootDescriptorTable(1, fragUHandle);

		commandList->SetPipelineState(graphicsPipeline.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		commandList->DrawInstanced(vertexCount, 1, 0, 0);
	}

	void ObjectCubeDX::cleanUpCube()
	{
		fragUniformAllocation->Release();
		vertUniformAllocation->Release();
		vertexAllocation->Release();
	}

	void ObjectCubeDX::reloadPipeline(ComPtr<ID3D12Device2> device, UINT msaaSamples)
	{
		if (enableWireframe)
		{
			enableWireframe = false;
		}
		else if (!enableWireframe)
		{
			enableWireframe = true;
		}

		createGraphicsPipeline(device, msaaSamples);
	}
}