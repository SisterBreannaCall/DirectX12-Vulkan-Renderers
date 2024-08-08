#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT

#include "NuklearDX.h"

static nk_font_atlas nAtlas;
static nk_font* nFont;
static enum nk_anti_aliasing AA;
static nk_buffer cmds;

namespace DirectX
{
	void NuklearDX::createNGui(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue, nk_context& nContext, int width, int height)
	{
		struct nk_font_config cfg = nk_font_config(0);
		cfg.oversample_h = 3;
		cfg.oversample_v = 2;

		nk_font_atlas_init_default(&nAtlas);
		nk_font_atlas_begin(&nAtlas);
		nFont = nk_font_atlas_add_default(&nAtlas, 14.0f, &cfg);

		createDescriptorHeap(device);
		createFont(device, allocator, queue);
		createGraphicsPipeline(device);

		nk_font_atlas_end(&nAtlas, nk_handle_ptr(srvDescriptorHeap.Get()), nullptr);
		nk_init_default(&nContext, &nFont->handle);
		nk_buffer_init_default(&cmds);
	}

	void NuklearDX::createDescriptorHeap(ComPtr<ID3D12Device2> device)
	{
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 1;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

		device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&srvDescriptorHeap));
	}

	void NuklearDX::createFont(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue)
	{
		const void* image;
		int texWidth, texHeight;

		image = nk_font_atlas_bake(&nAtlas, &texWidth, &texHeight, NK_FONT_ATLAS_RGBA32);

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
		textureData.pData = image;
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
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

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

	void NuklearDX::createGraphicsPipeline(ComPtr<ID3D12Device2> device)
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
			offsetof(NkDrawVert, pos),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},

			{"TEXCORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			offsetof(NkDrawVert, uv),
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0},
			
			{"COLOR",
			0,
			DXGI_FORMAT_R8G8B8A8_UNORM,
			0,
			offsetof(NkDrawVert, col),
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

		CD3DX12_ROOT_PARAMETER1 rootsParameters[2];
		rootsParameters[0].InitAsDescriptorTable(
			1,
			&ranges[0],
			D3D12_SHADER_VISIBILITY_PIXEL);
		rootsParameters[1].InitAsConstants(
			sizeof(pushConstBlock) / 4,
			0,
			0,
			D3D12_SHADER_VISIBILITY_VERTEX);

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
		rootSignatureDesc.Init_1_1(
			_countof(rootsParameters),
			rootsParameters,
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

	void NuklearDX::createVertexBuffer(D3D12MA::Allocator* allocator, size_t bufferSize)
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

	void NuklearDX::destroyVertexBuffer()
	{
		vertexAllocation->Release();
	}

	void NuklearDX::createIndexBuffer(D3D12MA::Allocator* allocator, size_t bufferSize)
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

	void NuklearDX::destroyIndexBuffer()
	{
		indexAllocation->Release();
	}

	void NuklearDX::updateNGui(D3D12MA::Allocator* allocator, nk_context& nContext)
	{
		nk_convert_config config;
		static const nk_draw_vertex_layout_element vertex_layout[] = {
			{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(NkDrawVert, pos)},
			{NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(NkDrawVert, uv)},
			{NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(NkDrawVert, col)},
			{NK_VERTEX_LAYOUT_END}
		};

		config.vertex_layout = vertex_layout;
		config.vertex_size = sizeof(NkDrawVert);
		config.vertex_alignment = NK_ALIGNOF(NkDrawVert);
		config.circle_segment_count = 22;
		config.curve_segment_count = 22;
		config.arc_segment_count = 22;
		config.global_alpha = 1.0f;
		config.shape_AA = AA;
		config.line_AA = AA;

		size_t vertexBufferSize = 512 * 1024;
		size_t indexBufferSize = 128 * 1024;

		if (vertexBuffer == NULL)
		{
			destroyVertexBuffer();
			createVertexBuffer(allocator, vertexBufferSize);
		}

		if (indexBuffer == NULL)
		{
			destroyIndexBuffer();
			createIndexBuffer(allocator, indexBufferSize);
		}

		void* vtxResouce;
		void* idxResouce;

		CD3DX12_RANGE readRange(0, 0);
		vertexBuffer->Map(0, &readRange, &vtxResouce);
		indexBuffer->Map(0, &readRange, &idxResouce);

		nk_buffer vBuffer;
		nk_buffer iBuffer;

		nk_buffer_init_fixed(&vBuffer, vtxResouce, vertexBufferSize);
		nk_buffer_init_fixed(&iBuffer, idxResouce, indexBufferSize);

		nk_convert(&nContext, &cmds, &vBuffer, &iBuffer, &config);

		vertexBuffer->Unmap(0, nullptr);
		indexBuffer->Unmap(0, nullptr);

		nk_buffer_free(&vBuffer);
		nk_buffer_free(&iBuffer);

		vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress() + 0;
		vertexBufferView.SizeInBytes = (UINT)vertexBufferSize;
		vertexBufferView.StrideInBytes = sizeof(NkDrawVert);

		indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
		indexBufferView.SizeInBytes = (UINT)indexBufferSize;
		indexBufferView.Format = DXGI_FORMAT_R16_UINT;
	}

	void NuklearDX::recordNGui(ComPtr<ID3D12GraphicsCommandList2> commandList, CD3DX12_CPU_DESCRIPTOR_HANDLE rtv, nk_context& nContext, float width, float height)
	{
		D3D12_VIEWPORT viewport;
		viewport.Width = width;
		viewport.Height = height;
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

		pushConstBlock.scale = glm::vec2(2.0f / width, 2.0f / height);
		pushConstBlock.translate = glm::vec2(-1.0f);
		commandList->SetGraphicsRoot32BitConstants(1, sizeof(PushConstBlock) / 4, &pushConstBlock, 0);
		commandList->SetPipelineState(graphicsPipeline.Get());
		commandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);

		int32_t vertexOffset = 0;
		int32_t indexOffset = 0;

		if (cmds.needed > 0)
		{
			const nk_draw_command* cmd;
			nk_draw_foreach(cmd, &nContext, &cmds)
			{
				D3D12_RECT scissorRect;

				if (!cmd->elem_count)
					continue;

				scissorRect.left = (LONG)cmd->clip_rect.x;
				scissorRect.right = (LONG)(cmd->clip_rect.x + cmd->clip_rect.w);
				scissorRect.top = (LONG)cmd->clip_rect.y;
				scissorRect.bottom = (LONG)(cmd->clip_rect.y + cmd->clip_rect.h);

				commandList->RSSetScissorRects(1, &scissorRect);
				commandList->DrawIndexedInstanced(cmd->elem_count, 1, indexOffset, vertexOffset, 0);
				indexOffset += cmd->elem_count;
			}
		}
	}

	void NuklearDX::cleanUpNGui(nk_context& nContext)
	{
		destroyIndexBuffer();
		destroyVertexBuffer();
		fontAllocation->Release();
		nk_buffer_free(&cmds);
		nk_font_atlas_clear(&nAtlas);
		nk_free(&nContext);
	}

	void NuklearDX::nGuiFrame(GLFWwindow* window, nk_context& nContext)
	{
		nk_clear(&nContext);
		nk_buffer_clear(&cmds);
		nk_style_set_font(&nContext, &nFont->handle);

		nk_input_begin(&nContext);
		double mouse_x, mouse_y;
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
}