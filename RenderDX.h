#ifndef RENDERDX_H
#define RENDERDX_H

#include <wrl.h>
using namespace Microsoft::WRL;

#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/glfw3native.h>

#if defined(max)
#undef max
#endif

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dx12/d3dx12.h>
#include <d3dcompiler.h>

#include "QueueDX.h"

#include <glm/glm.hpp>

#include "DMA/D3D12MemAlloc.h"

#include <vector>
#include <chrono>
#include <iostream>
#include <memory>
#include <algorithm>

namespace DirectX
{
	class RenderDX
	{
	private:
		HWND hWnd;
		bool vSync = true;
		bool tearingSupported = false;
		bool isInitialized = false;
		static const uint8_t numFrames = 3;
		ComPtr<IDXGIAdapter1> adapter;
		ComPtr<IDXGISwapChain4> swapChain;
		ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
		ComPtr<ID3D12Resource> multiBuffer;
		D3D12MA::Allocation* multiAllocation = nullptr;
		ComPtr<ID3D12DescriptorHeap> multiDescriptorHeap;
		ComPtr<ID3D12Resource> depthBuffer;
		D3D12MA::Allocation* depthAllocation = nullptr;
		ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
		UINT rtvDescriptorSize;
		UINT currentBackBufferIndex;
		ComPtr<ID3D12Resource> backBuffers[numFrames];
		uint64_t frameFenceValues[numFrames] = {};

		void enableDebugLayer();
		void checkTearingSupport();
		void createDevice();
		void createMemoryAllocator();
		void createSwapChain();
		void createDescriptorHeaps();
		void createRenderTargetViews();
		void createCommandAllocators();
		void createColorResources();
		void createDepthResources();

	public:
		int width;
		int height;
		bool enableDebug;
		UINT msaaSamples;
		std::vector<float> clearColor;
		ComPtr<ID3D12Device2> device;
		D3D12MA::Allocator* allocator;
		std::shared_ptr<QueueDX> directCommandQueue;
		std::shared_ptr<QueueDX> computeCommandQueue;
		std::shared_ptr<QueueDX> copyCommandQueue;
		CD3DX12_CPU_DESCRIPTOR_HANDLE gRtv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE gMtv;
		CD3DX12_CPU_DESCRIPTOR_HANDLE gDsv;
		ComPtr<ID3D12GraphicsCommandList2> gCommandList;
		ComPtr<ID3D12Resource> gBackBuffer;

		void initDX(GLFWwindow* window);
		void flushDevice();
		void beginRecord();
		void resolveMulti();
		void endRecord();
		void cleanUpDX();
		void recreateSwapChain(int vWidth, int vHeight);
		void recreate();
	};
}
#endif