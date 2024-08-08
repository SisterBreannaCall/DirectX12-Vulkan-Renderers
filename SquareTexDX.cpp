#include "SquareTexDX.h"

namespace DirectX
{
	void SquareTexDX::createSquare(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
	{
		// Make sure to use direct queue instead of copy queue.

		vertices = {
			{{ 0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
			{{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
			{{-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
			{{-0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
		};

		indices = {
			0, 1, 3, 1, 2, 3
		};

		textureFileName = "data/images/container.jpg";

		createVertexBuffer(device, allocator, queue);
		createIndexBuffer(device, allocator, queue);
		createDescriptorHeap(device);
		createTexture(device, allocator, queue);
		createGraphicsPipeline(device);
	}

	void SquareTexDX::createVertexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
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
		vertexBufferView.StrideInBytes = sizeof(Vertex2D);

		auto fenceValue = queue->executeCommandList(commandList);
		queue->waitForFenceValue(fenceValue);

		stagingAllocation->Release();
	}

	void SquareTexDX::createIndexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
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

	void SquareTexDX::createDescriptorHeap(ComPtr<ID3D12Device2> device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	}

	void SquareTexDX::createTexture(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
	{
		auto commandList = queue->getCommandList(device);
		int imageWidth, imageHeight, imageChannels;

		stbi_uc* pixels = stbi_load(
			textureFileName.c_str(),
			&imageWidth,
			&imageHeight,
			&imageChannels,
			STBI_rgb_alpha);

		size_t imageSize = imageWidth * imageHeight * 4;

		D3D12_RESOURCE_DESC stagingBufferDesc = {};
		stagingBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		stagingBufferDesc.Width = imageSize;
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

		D3D12_RESOURCE_DESC textureDesc = {};
		textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		textureDesc.Alignment = 0;
		textureDesc.Width = imageWidth;
		textureDesc.Height = imageHeight;
		textureDesc.DepthOrArraySize = 1;
		textureDesc.MipLevels = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12MA::ALLOCATION_DESC textureAllocDesc = {};
		textureAllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		allocator->CreateResource(
			&textureAllocDesc,
			&textureDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			&imageAllocation,
			IID_PPV_ARGS(&imageBuffer));

		D3D12_SUBRESOURCE_DATA imageData = {};
		imageData.pData = pixels;
		imageData.RowPitch = imageWidth * 4;
		imageData.SlicePitch = imageData.RowPitch * imageHeight;

		UpdateSubresources(
			commandList.Get(),
			imageBuffer.Get(),
			stagingBuffer.Get(),
			0,
			0,
			1,
			&imageData);

		stbi_image_free(pixels);

		D3D12_RESOURCE_BARRIER imageBarrier = {};
		imageBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		imageBarrier.Transition.pResource = imageBuffer.Get();
		imageBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		imageBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		imageBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		commandList->ResourceBarrier(1, &imageBarrier);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		device->CreateShaderResourceView(
			imageBuffer.Get(),
			&srvDesc,
			srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		auto fenceValue = queue->executeCommandList(commandList);
		queue->waitForFenceValue(fenceValue);

		stagingAllocation->Release();
	}

	void SquareTexDX::createGraphicsPipeline(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3DBlob> vertShaderBlob;
		ComPtr<ID3DBlob> fragShaderBlob;

		D3DReadFileToBlob(L"data/shadersdx/DX2DTEXV.cso", &vertShaderBlob);
		D3DReadFileToBlob(L"data/shadersdx/DX2DTEXF.cso", &fragShaderBlob);

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			offsetof(Vertex2D, position),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},

			{"COLOR",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(Vertex2D, color),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},

			{"TEXCORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			offsetof(Vertex2D, texCoord),
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

		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
		ranges[0].Init(
			D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
			1,
			0,
			0,
			D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

		CD3DX12_ROOT_PARAMETER1 rootParameters[2];
		rootParameters[0].InitAsDescriptorTable(
			1,
			&ranges[0],
			D3D12_SHADER_VISIBILITY_PIXEL);
		rootParameters[1].InitAsConstants(
			sizeof(pushConstBlock) / 4,
			0,
			0,
			D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init_1_1(
			_countof(rootParameters),
			rootParameters,
			1,
			&sampler,
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
		graphicsDesc.DepthStencilState.DepthEnable = FALSE;
		graphicsDesc.DepthStencilState.StencilEnable = FALSE;
		graphicsDesc.SampleMask = UINT_MAX;
		graphicsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		graphicsDesc.NumRenderTargets = 1;
		graphicsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		graphicsDesc.SampleDesc.Count = 1;

		device->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(&graphicsPipeline));
	}

	void SquareTexDX::recordSquare(ComPtr<ID3D12GraphicsCommandList2> commandList, CD3DX12_CPU_DESCRIPTOR_HANDLE rtv, int width, int height)
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

		ID3D12DescriptorHeap* ppHeaps[] = { srvDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(0, srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		pushConstBlock.translate = glm::mat4(1.0f);
		pushConstBlock.translate = glm::rotate(pushConstBlock.translate, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, -1.0f));
		commandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstBlock) / 4, &pushConstBlock, 0);

		commandList->SetPipelineState(graphicsPipeline.Get());
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);
		commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
		commandList->DrawIndexedInstanced(indexCount, 1, 0, 0, 0);
	}

	void SquareTexDX::cleanUpSquare()
	{
		imageAllocation->Release();
		indexAllocation->Release();
		vertexAllocation->Release();
	}

	void SquareTexDX::reloadPipeline(ComPtr<ID3D12Device2> device)
	{
		createGraphicsPipeline(device);
	}
}