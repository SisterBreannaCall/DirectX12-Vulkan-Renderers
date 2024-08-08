#include "CubeDX.h"

namespace DirectX
{
	void CubeDX::createCube(ComPtr<ID3D12Device2> device, D3D12MA::Allocator * allocator, std::shared_ptr<QueueDX> queue)
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

		createVertexBuffer(device, allocator, queue);
		createIndexBuffer(device, allocator, queue);
		createDescriptorHeap(device);
		createUniformBuffer(device, allocator);
		createGraphicsPipeline(device);
	}

	void CubeDX::createVertexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
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

	void CubeDX::createIndexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
	{
		size_t bufferSize = sizeof(indices[0]) * indices.size();
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
		memcpy(data, indices.data(), bufferSize);
		stagingBuffer->Unmap(0, nullptr);

		indexCount = (UINT)indices.size();
		indices.clear();
		indices.shrink_to_fit();

		D3D12_RESOURCE_DESC indexBufferDesc = {};
		indexBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		indexBufferDesc.Alignment = 0;
		indexBufferDesc.Width = bufferSize;
		indexBufferDesc.Height = 1;
		indexBufferDesc.DepthOrArraySize = 1;
		indexBufferDesc.MipLevels = 1;
		indexBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		indexBufferDesc.SampleDesc.Count = 1;
		indexBufferDesc.SampleDesc.Quality = 0;
		indexBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		indexBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC indexAllocDesc = {};
		indexAllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		allocator->CreateResource(
			&indexAllocDesc,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			&indexAllocation,
			IID_PPV_ARGS(&indexBuffer));

		commandList->CopyResource(indexBuffer.Get(), stagingBuffer.Get());

		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = (UINT)bufferSize;
		indexBufferView.Format = DXGI_FORMAT_R16_UINT;

		auto fenceValue = queue->executeCommandList(commandList);
		queue->waitForFenceValue(fenceValue);

		stagingAllocation->Release();
	}

	void CubeDX::createDescriptorHeap(ComPtr<ID3D12Device2> device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&uniformDescriptorHeap));
	}

	void CubeDX::createUniformBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator)
	{
		size_t bufferSize = sizeof(UniformData);

		D3D12_RESOURCE_DESC uniformBufferDesc = {};
		uniformBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		uniformBufferDesc.Alignment = 0;
		uniformBufferDesc.Width = bufferSize;
		uniformBufferDesc.Height = 1;
		uniformBufferDesc.DepthOrArraySize = 1;
		uniformBufferDesc.MipLevels = 1;
		uniformBufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		uniformBufferDesc.SampleDesc.Count = 1;
		uniformBufferDesc.SampleDesc.Quality = 0;
		uniformBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		uniformBufferDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC uniformAllocDesc = {};
		uniformAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		allocator->CreateResource(
			&uniformAllocDesc,
			&uniformBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&uniformAllocation,
			IID_PPV_ARGS(&uniformBuffer));

		D3D12_CONSTANT_BUFFER_VIEW_DESC uniformViewDesc = {};
		uniformViewDesc.BufferLocation = uniformBuffer->GetGPUVirtualAddress();
		uniformViewDesc.SizeInBytes = (UINT)bufferSize;
		device->CreateConstantBufferView(
			&uniformViewDesc, 
			uniformDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void CubeDX::createGraphicsPipeline(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3DBlob> vertShaderBlob;
		ComPtr<ID3DBlob> fragShaderBlob;

		D3DReadFileToBlob(L"data/shadersdx/DXCUBEV.cso", &vertShaderBlob);
		D3DReadFileToBlob(L"data/shadersdx/DX2DF.cso", &fragShaderBlob);

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION", 
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(Vertex3D, position),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},
			
			{"COLOR",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(Vertex3D, color),
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

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_CBV,
			1,
			0,
			0,
			D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[1];
		rootParameters[0].InitAsDescriptorTable(
			1, 
			&ranges[0],
			D3D12_SHADER_VISIBILITY_VERTEX);

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
		graphicsDesc.SampleDesc.Count = 1;

		device->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(&graphicsPipeline));
	}

	void CubeDX::updateCube(glm::mat4 viewMatrix, int width, int height)
	{
		uniformData.model = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
		uniformData.view = glm::mat4(1.0f);
		uniformData.view = viewMatrix;
		uniformData.projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

		void* data;
		CD3DX12_RANGE readRange(0, 0);
		uniformBuffer->Map(0, &readRange, &data);
		memcpy(data, &uniformData, sizeof(UniformData));
		uniformBuffer->Unmap(0, nullptr);
	}

	void CubeDX::recordCube(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv, int width, int height)
	{
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

		ID3D12DescriptorHeap* ppHeaps[] = { uniformDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(0, uniformDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		commandList->SetPipelineState(graphicsPipeline.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);
		commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
		commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);
		commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}

	void CubeDX::cleanUpCube()
	{
		uniformAllocation->Release();
		indexAllocation->Release();
		vertexAllocation->Release();
	}

	void CubeDX::reloadPipeline(ComPtr<ID3D12Device2> device)
	{
		createGraphicsPipeline(device);
	}
}