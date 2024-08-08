#ifndef SQUARECONSTANTDX_H
#define SQUARECONSTANTDX_H

#include <wrl.h>
using namespace Microsoft::WRL;

#include <GLFW/glfw3.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dx12/d3dx12.h>
#include <d3dcompiler.h>

#include "QueueDX.h"

#include <DMA/D3D12MemAlloc.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <array>
#include <algorithm>

namespace DirectX
{
	class SquareConstantDX
	{
		struct Vertex2D
		{
			glm::vec2 position;
			glm::vec3 color;
		};

		struct PushConstBlock
		{
			glm::mat4 translate;
		} pushConstBlock;

	private:
		ComPtr<ID3D12Resource> vertexBuffer;
		D3D12MA::Allocation* vertexAllocation = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		ComPtr<ID3D12Resource> indexBuffer;
		D3D12MA::Allocation* indexAllocation = nullptr;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		UINT indexCount = 0;
		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> graphicsPipeline;

		void createVertexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void createIndexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void createGraphicsPipeline(ComPtr<ID3D12Device2> device);

	public:
		std::vector<Vertex2D> vertices;
		std::vector<uint16_t> indices;
		bool enableWireframe = false;

		void createSquare(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void cleanUpSquare();
		void recordSquare(ComPtr<ID3D12GraphicsCommandList2> commandList, CD3DX12_CPU_DESCRIPTOR_HANDLE rtv, int width, int height);
		void reloadPipeline(ComPtr<ID3D12Device2> device);
	};
}
#endif