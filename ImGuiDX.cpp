#include "ImGuiDX.h"

namespace DirectX
{
	void ImGuiDX::createIGui(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue, int width, int height)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui::StyleColorsDark();
		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

		createDescriptorHeap(device);
		createFont(device, allocator, queue);
		createGraphicsPipeline(device);
	}

	void ImGuiDX::createDescriptorHeap(ComPtr<ID3D12Device2> device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	}

	void ImGuiDX::createFont(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
	{
		unsigned char* fontData;
		int texWidth, texHeight;
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

		auto commandList = queue->getCommandList(device);

		UINT uploadPitch = (texWidth * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT uploadSize = texHeight * uploadPitch;

		D3D12_RESOURCE_DESC stagingBufferDesc = {};
		stagingBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		stagingBufferDesc.Alignment = 0;
		stagingBufferDesc.Width = uploadSize;
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
		textureDesc.Width = texWidth;
		textureDesc.Height = texHeight;
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
			&fontAllocation,
			IID_PPV_ARGS(&fontBuffer));

		D3D12_SUBRESOURCE_DATA textureData = {};
		textureData.pData = fontData;
		textureData.RowPitch = uploadPitch;
		textureData.SlicePitch = uploadSize;

		UpdateSubresources(
			commandList.Get(),
			fontBuffer.Get(),
			stagingBuffer.Get(),
			0,
			0,
			1,
			&textureData);

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Transition.pResource = fontBuffer.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		commandList->ResourceBarrier(1, &barrier);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		device->CreateShaderResourceView(
			fontBuffer.Get(),
			&srvDesc,
			srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		auto fenceValue = queue->executeCommandList(commandList);
		queue->waitForFenceValue(fenceValue);

		stagingAllocation->Release();
	}

	void ImGuiDX::createGraphicsPipeline(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3DBlob> vertShaderBlob;
		ComPtr<ID3DBlob> fragShaderBlob;

		D3DReadFileToBlob(L"data/shadersdx/UIV.cso", &vertShaderBlob);
		D3DReadFileToBlob(L"data/shadersdx/UIF.cso", &fragShaderBlob);

		D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
			{"POSITION",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			offsetof(ImDrawVert, pos),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},

			{"TEXCORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			offsetof(ImDrawVert, uv),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},

			{"COLOR",
			0,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			0,
			offsetof(ImDrawVert, col),
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
		rasterizer.FillMode = D3D12_FILL_MODE_SOLID;
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
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MipLODBias = 0;
		sampler.MaxAnisotropy = 0;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_BLEND_DESC blendDesc = {};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

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

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
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
		graphicsDesc.BlendState = blendDesc;
		graphicsDesc.DepthStencilState.DepthEnable = FALSE;
		graphicsDesc.DepthStencilState.StencilEnable = FALSE;
		graphicsDesc.SampleMask = UINT_MAX;
		graphicsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		graphicsDesc.NumRenderTargets = 1;
		graphicsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		graphicsDesc.SampleDesc.Count = 1;

		device->CreateGraphicsPipelineState(&graphicsDesc, IID_PPV_ARGS(&graphicsPipeline));
	}

	void ImGuiDX::createVertexBuffer(D3D12MA::Allocator* allocator, size_t bufferSize)
	{
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
		vertexAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		allocator->CreateResource(
			&vertexAllocDesc,
			&vertexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&vertexAllocation,
			IID_PPV_ARGS(&vertexBuffer));
	}

	void ImGuiDX::destroyVertexBuffer()
	{
		vertexAllocation->Release();
	}

	void ImGuiDX::createIndexBuffer(D3D12MA::Allocator* allocator, size_t bufferSize)
	{
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
		indexAllocDesc.HeapType = D3D12_HEAP_TYPE_UPLOAD;

		allocator->CreateResource(
			&indexAllocDesc,
			&indexBufferDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			&indexAllocation,
			IID_PPV_ARGS(&indexBuffer));
	}

	void ImGuiDX::destroyIndexBuffer()
	{
		indexAllocation->Release();
	}

	void ImGuiDX::updateIGui(D3D12MA::Allocator* allocator)
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();

		size_t vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		size_t indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

		if ((vertexBufferSize == 0) || (indexBufferSize == 0))
		{
			return;
		}

		if ((vertexBuffer == NULL) || (vertexCount != imDrawData->TotalVtxCount))
		{
			destroyVertexBuffer();
			createVertexBuffer(allocator, vertexBufferSize);
			vertexCount = imDrawData->TotalVtxCount;
		}

		if ((indexBuffer == NULL) || (indexCount < imDrawData->TotalIdxCount))
		{
			destroyIndexBuffer();
			createIndexBuffer(allocator, indexBufferSize);
			indexCount = imDrawData->TotalIdxCount;
		}

		void* vtxResource;
		void* idxResource;

		CD3DX12_RANGE readRange(0, 0);
		vertexBuffer->Map(0, &readRange, &vtxResource);
		indexBuffer->Map(0, &readRange, &idxResource);

		ImDrawVert* vtxDst = (ImDrawVert*)vtxResource;
		ImDrawIdx* idxDst = (ImDrawIdx*)idxResource;

		for (int i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			idxDst += cmd_list->IdxBuffer.Size;
		}

		vertexBuffer->Unmap(0, nullptr);
		indexBuffer->Unmap(0, nullptr);

		unsigned int stride = sizeof(ImDrawVert);
		unsigned int offset = 0;

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + offset;
		vertexBufferView.SizeInBytes = vertexCount * stride;
		vertexBufferView.StrideInBytes = stride;

		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = indexCount * sizeof(ImDrawIdx);
		indexBufferView.Format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	}

	void ImGuiDX::iGuiFrame(GLFWwindow* window)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
		{
			double mouse_x;
			double mouse_y;
			glfwGetCursorPos(window, &mouse_x, &mouse_y);
			io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
		}

		else
		{
			io.MousePos = ImVec2(-1, -1);
		}

		for (int i = 0; i < 3; i++)
		{
			io.MouseDown[i] = glfwGetMouseButton(window, i) != 0;
		}

		glfwSetInputMode(window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
	}

	void ImGuiDX::recordIGui(ComPtr<ID3D12GraphicsCommandList2> commandList, CD3DX12_CPU_DESCRIPTOR_HANDLE rtv)
	{
		ImGuiIO& io = ImGui::GetIO();

		D3D12_VIEWPORT viewport;
		viewport.Width = io.DisplaySize.x;
		viewport.Height = io.DisplaySize.y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		commandList->RSSetViewports(1, &viewport);

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		commandList->IASetIndexBuffer(&indexBufferView);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->SetGraphicsRootSignature(rootSignature.Get());

		ID3D12DescriptorHeap* ppHeaps[] = { srvDescriptorHeap.Get() };
		commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
		commandList->SetGraphicsRootDescriptorTable(0, srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

		pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		pushConstBlock.translate = glm::vec2(-1.0f);
		commandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstBlock) / 4, &pushConstBlock, 0);
		commandList->SetPipelineState(graphicsPipeline.Get());
		commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

		ImDrawData* imDrawData = ImGui::GetDrawData();
		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;
		ImVec2 clip_off = imDrawData->DisplayPos;

		if (imDrawData->CmdListsCount > 0)
		{
			for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
			{
				const ImDrawList* cmd_list = imDrawData->CmdLists[i];
				for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
					const D3D12_RECT scissorRect = {
						((LONG)(pcmd->ClipRect.x - clip_off.x)),
						((LONG)(pcmd->ClipRect.y - clip_off.y)),
						((LONG)(pcmd->ClipRect.z - clip_off.x)),
						((LONG)(pcmd->ClipRect.w - clip_off.y))
					};

					commandList->RSSetScissorRects(1, &scissorRect);
					commandList->DrawIndexedInstanced(pcmd->ElemCount, 1, pcmd->IdxOffset + indexOffset, pcmd->VtxOffset + vertexOffset, 0);
				}
				indexOffset += cmd_list->IdxBuffer.Size;
				vertexOffset += cmd_list->VtxBuffer.Size;
			}
		}
	}

	void ImGuiDX::cleanUpIGui()
	{
		destroyIndexBuffer();
		destroyVertexBuffer();
		fontAllocation->Release();
		ImGui::DestroyContext();
	}
}