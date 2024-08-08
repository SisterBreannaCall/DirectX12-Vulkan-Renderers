#include "RenderDX.h"

namespace DirectX
{
	void RenderDX::initDX(GLFWwindow* window)
	{
		hWnd = glfwGetWin32Window(window);

		enableDebugLayer();
		checkTearingSupport();
		createDevice();
		createMemoryAllocator();
		createCommandAllocators();
		createSwapChain();
		createDescriptorHeaps();
		createRenderTargetViews();
		createColorResources();
		createDepthResources();

		if (enableDebug)
		{
			std::cout << "SkyNet :: directX 12 initialized" << std::endl;
		}
	}

	void RenderDX::enableDebugLayer()
	{
		if (enableDebug)
		{
			ComPtr<ID3D12Debug> debugInterface;
			D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
			debugInterface->EnableDebugLayer();
		}
	}

	void RenderDX::checkTearingSupport()
	{
		ComPtr<IDXGIFactory4> factory4;
		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
		{
			ComPtr<IDXGIFactory5> factory5;
			if (SUCCEEDED(factory4.As(&factory5)))
			{
				if (FAILED(factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &tearingSupported, sizeof(tearingSupported))))
				{
					tearingSupported = false;
					return;
				}
			}
		}

		tearingSupported = true;
	}

	void RenderDX::createDevice()
	{
		ComPtr<IDXGIFactory4> dxgiFactory;
		UINT createFactoryFlags = 0;

		if (enableDebug)
		{
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		}

		CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory));

		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		size_t maxDedicatedVideoMemory = 0;
		for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++)
		{
			DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
			adapter->GetDesc1(&dxgiAdapterDesc1);

			if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 && SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr)) && dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
			{
				maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
				adapter.As(&dxgiAdapter4);
			}
		}

		adapter = dxgiAdapter4;

		D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

		if (enableDebug)
		{
			ComPtr<ID3D12InfoQueue> pInfoQueue;
			if (SUCCEEDED(device.As(&pInfoQueue)))
			{
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

				D3D12_MESSAGE_SEVERITY Severities[] =
				{
					D3D12_MESSAGE_SEVERITY_INFO
				};

				D3D12_MESSAGE_ID DenyIds[] = {
					D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
					D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,
					D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,
				};

				D3D12_INFO_QUEUE_FILTER NewFilter = {};
				NewFilter.DenyList.NumSeverities = _countof(Severities);
				NewFilter.DenyList.pSeverityList = Severities;
				NewFilter.DenyList.NumIDs = _countof(DenyIds);
				NewFilter.DenyList.pIDList = DenyIds;

				pInfoQueue->PushStorageFilter(&NewFilter);
			}
		}
	}

	void RenderDX::createMemoryAllocator()
	{
		D3D12MA::ALLOCATOR_DESC allocatorDesc = {};
		allocatorDesc.pDevice = device.Get();
		allocatorDesc.pAdapter = adapter.Get();

		D3D12MA::CreateAllocator(&allocatorDesc, &allocator);
	}

	void RenderDX::createSwapChain()
	{
		ComPtr<IDXGIFactory4> dxgiFactory4;

		UINT createFactoryFlags = 0;

		if (enableDebug)
		{
			createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
		}

		CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4));

		DXGI_SWAP_CHAIN_DESC1 createInfo = {};
		createInfo.Width = width;
		createInfo.Height = height;
		createInfo.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		createInfo.Stereo = FALSE;
		createInfo.SampleDesc = { 1, 0 };
		createInfo.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		createInfo.BufferCount = numFrames;
		createInfo.Scaling = DXGI_SCALING_STRETCH;
		createInfo.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		createInfo.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		createInfo.Flags = tearingSupported ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ID3D12CommandQueue* pCommandQueue = directCommandQueue->getD3D12CommandQueue().Get();

		ComPtr<IDXGISwapChain1> swapChain1;
		dxgiFactory4->CreateSwapChainForHwnd(pCommandQueue, hWnd, &createInfo, nullptr, nullptr, &swapChain1);

		dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
		swapChain1.As(&swapChain);

		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();
	}

	void RenderDX::createDescriptorHeaps()
	{
		// Swap chain RTV
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = numFrames;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

		device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
		rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		// MultiSample Heap
		D3D12_DESCRIPTOR_HEAP_DESC multiHeapDesc = {};
		multiHeapDesc.NumDescriptors = 1;
		multiHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		device->CreateDescriptorHeap(&multiHeapDesc, IID_PPV_ARGS(&multiDescriptorHeap));

		// Depth Stencil Heap
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap));
	}

	void RenderDX::createRenderTargetViews()
	{
		auto rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		for (int i = 0; i < numFrames; i++)
		{
			ComPtr<ID3D12Resource> backBuffer;
			swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));

			device->CreateRenderTargetView(backBuffer.Get(), nullptr, rtvHandle);

			backBuffers[i] = backBuffer;

			rtvHandle.Offset(rtvDescriptorSize);
		}
	}

	void RenderDX::createCommandAllocators()
	{
		directCommandQueue = std::make_shared<QueueDX>(device, D3D12_COMMAND_LIST_TYPE_DIRECT);
		computeCommandQueue = std::make_shared<QueueDX>(device, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		copyCommandQueue = std::make_shared<QueueDX>(device, D3D12_COMMAND_LIST_TYPE_COPY);
	}

	void RenderDX::createColorResources()
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msLevels;
		msLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;

		for (int i = 1; i < 32; i)
		{
			msLevels.SampleCount = i;

			device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msLevels, sizeof(msLevels));

			if (msLevels.NumQualityLevels == 1)
			{
				msaaSamples = i;
			}

			i = i * 2;
		}

		device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msLevels, sizeof(msLevels));

		D3D12_RESOURCE_DESC multiSampDesc = {};
		multiSampDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		multiSampDesc.Alignment = 0;
		multiSampDesc.Width = width;
		multiSampDesc.Height = height;
		multiSampDesc.DepthOrArraySize = 1;
		multiSampDesc.MipLevels = 1;
		multiSampDesc.Format = msLevels.Format;
		multiSampDesc.SampleDesc.Count = msaaSamples;
		multiSampDesc.SampleDesc.Quality = msLevels.NumQualityLevels;
		multiSampDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		multiSampDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

		D3D12MA::ALLOCATION_DESC multiAllocDesc = {};
		multiAllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		CD3DX12_CLEAR_VALUE multiClearValue = {};
		multiClearValue.Format = msLevels.Format;
		memcpy(multiClearValue.Color, clearColor.data(), clearColor.size());

		allocator->CreateResource(
			&multiAllocDesc,
			&multiSampDesc,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
			&multiClearValue,
			&multiAllocation,
			IID_PPV_ARGS(&multiBuffer));

		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = msLevels.Format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;

		gMtv = multiDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		device->CreateRenderTargetView(multiBuffer.Get(), &rtvDesc, gMtv);
	}

	void RenderDX::createDepthResources()
	{
		D3D12_RESOURCE_DESC depthBufferDesc = {};
		depthBufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthBufferDesc.Alignment = 0;
		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.DepthOrArraySize = 1;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthBufferDesc.SampleDesc.Count = msaaSamples;
		depthBufferDesc.SampleDesc.Quality = 0;
		depthBufferDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		depthBufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12MA::ALLOCATION_DESC depthAllocDesc = {};
		depthAllocDesc.HeapType = D3D12_HEAP_TYPE_DEFAULT;

		CD3DX12_CLEAR_VALUE clearValueDepth = {};
		clearValueDepth.Format = DXGI_FORMAT_D32_FLOAT;
		clearValueDepth.DepthStencil.Depth = 1.0f;
		clearValueDepth.DepthStencil.Stencil = 0;

		allocator->CreateResource(
			&depthAllocDesc,
			&depthBufferDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValueDepth,
			&depthAllocation,
			IID_PPV_ARGS(&depthBuffer));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvViewDesc = {};
		dsvViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
		dsvViewDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvViewDesc.Texture2D.MipSlice = 0;

		gDsv = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

		device->CreateDepthStencilView(depthBuffer.Get(), &dsvViewDesc, gDsv);
	}

	void RenderDX::flushDevice()
	{
		directCommandQueue->flush();
		computeCommandQueue->flush();
		copyCommandQueue->flush();
	}

	void RenderDX::beginRecord()
	{
		auto commandList = directCommandQueue->getCommandList(device);
		auto backBuffer = backBuffers[currentBackBufferIndex];

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), currentBackBufferIndex, rtvDescriptorSize);

		// clear render targets
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				backBuffer.Get(),
				D3D12_RESOURCE_STATE_PRESENT, 
				D3D12_RESOURCE_STATE_RENDER_TARGET);

			commandList->ResourceBarrier(1, &barrier);

			barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				multiBuffer.Get(),
				D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
				D3D12_RESOURCE_STATE_RENDER_TARGET);

			commandList->ResourceBarrier(1, &barrier);

			commandList->ClearRenderTargetView(rtv, clearColor.data(), 0, nullptr);
			commandList->ClearRenderTargetView(gMtv, clearColor.data(), 0, nullptr);
			commandList->ClearDepthStencilView(gDsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);
		}

		gRtv = rtv;
		gCommandList = commandList;
		gBackBuffer = backBuffer;
	}

	void RenderDX::resolveMulti()
	{
		D3D12_RESOURCE_BARRIER barriers[2];

		barriers[0] = CD3DX12_RESOURCE_BARRIER::Transition(
			multiBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RESOLVE_SOURCE);

		barriers[1] = CD3DX12_RESOURCE_BARRIER::Transition(
			gBackBuffer.Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_RESOLVE_DEST);

		gCommandList->ResourceBarrier(2, barriers);

		gCommandList->ResolveSubresource(gBackBuffer.Get(), 0, multiBuffer.Get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			gBackBuffer.Get(),
			D3D12_RESOURCE_STATE_RESOLVE_DEST, 
			D3D12_RESOURCE_STATE_RENDER_TARGET);

		gCommandList->ResourceBarrier(1, &barrier);
	}

	void RenderDX::endRecord()
	{
		// present render targets
		{
			CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
				gBackBuffer.Get(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

			gCommandList->ResourceBarrier(1, &barrier);
			frameFenceValues[currentBackBufferIndex] = directCommandQueue->executeCommandList(gCommandList);

			UINT syncInterval = vSync ? 1 : 0;
			INT presentFlags = tearingSupported && !vSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
			swapChain->Present(syncInterval, presentFlags);

			currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

			directCommandQueue->waitForFenceValue(frameFenceValues[currentBackBufferIndex]);
		}
	}

	void RenderDX::cleanUpDX()
	{
		depthAllocation->Release();
		multiAllocation->Release();
		allocator->Release();
	}

	void RenderDX::recreateSwapChain(int vWidth, int vHeight)
	{
		width = vWidth;
		height = vHeight;

		for (int i = 0; i < numFrames; i++)
		{
			backBuffers[i].Reset();
			frameFenceValues[i] = frameFenceValues[currentBackBufferIndex];
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		swapChain->GetDesc(&swapChainDesc);
		swapChain->ResizeBuffers(numFrames, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags);

		currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

		createRenderTargetViews();
	}

	void RenderDX::recreate()
	{
		flushDevice();
		recreateSwapChain(width, height);
	}
}