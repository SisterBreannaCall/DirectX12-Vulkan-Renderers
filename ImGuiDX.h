#ifndef IMGUIDX_H
#define IMGUIDX_H

#include <wrl.h>
using namespace Microsoft::WRL;

#include <GLFW/glfw3.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dx12/d3dx12.h>
#include <d3dcompiler.h>

#include <IMGUI/imgui.h>

#include "QueueDX.h"

#include <DMA/D3D12MemAlloc.h>

#include <glm/glm.hpp>

#include <vector>
#include <array>
#include <algorithm>

namespace DirectX
{
	class ImGuiDX
	{
		struct PushConstBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

	private:
		ComPtr<ID3D12Resource> vertexBuffer;
		D3D12MA::Allocation* vertexAllocation = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		int32_t vertexCount = 0;
		ComPtr<ID3D12Resource> indexBuffer;
		D3D12MA::Allocation* indexAllocation = nullptr;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		int32_t indexCount = 0;
		ComPtr<ID3D12Resource> fontBuffer;
		D3D12MA::Allocation* fontAllocation = nullptr;
		ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> graphicsPipeline;

		void createDescriptorHeap(ComPtr<ID3D12Device2> device);
		void createFont(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void createGraphicsPipeline(ComPtr<ID3D12Device2> device);
		void createVertexBuffer(D3D12MA::Allocator* allocator, size_t bufferSize);
		void destroyVertexBuffer();
		void createIndexBuffer(D3D12MA::Allocator* allocator, size_t bufferSize);
		void destroyIndexBuffer();

	public:
		void createIGui(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue, int width, int height);
		void cleanUpIGui();
		void updateIGui(D3D12MA::Allocator* allocator);
		void recordIGui(ComPtr<ID3D12GraphicsCommandList2> commandList, CD3DX12_CPU_DESCRIPTOR_HANDLE rtv);
		void iGuiFrame(GLFWwindow* window);
	};
}
#endif