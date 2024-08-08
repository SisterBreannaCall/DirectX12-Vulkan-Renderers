#ifndef CUBETEXDX_H
#define CUBETEXDX_H

#include <wrl.h>
using namespace Microsoft::WRL;

#include <GLFW/glfw3.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dx12/d3dx12.h>
#include <d3dcompiler.h>

#include "QueueDX.h"

#include <DMA/D3D12MemAlloc.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <STB/stb_image.h>

#include <vector>
#include <array>
#include <algorithm>

namespace DirectX
{
	class CubeTexDX
	{
		struct Vertex3D
		{
			glm::vec3 position;
			glm::vec3 color;
			glm::vec2 texCoord;
		};

		struct UniformData
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
			float padding[16];
		} uniformData;

	private:
		ComPtr<ID3D12Resource> vertexBuffer;
		D3D12MA::Allocation* vertexAllocation = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		ComPtr<ID3D12Resource> indexBuffer;
		D3D12MA::Allocation* indexAllocation = nullptr;
		D3D12_INDEX_BUFFER_VIEW indexBufferView;
		UINT indexCount = 0;
		ComPtr<ID3D12DescriptorHeap> objectDescriptorHeap;
		UINT cbvSrvDescriptorSize;
		CD3DX12_CPU_DESCRIPTOR_HANDLE cbvSrvDescriptorHandle;
		ComPtr<ID3D12Resource> imageBuffer;
		D3D12MA::Allocation* imageAllocation = nullptr;
		ComPtr<ID3D12Resource> uniformBuffer;
		D3D12MA::Allocation* uniformAllocation = nullptr;
		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> graphicsPipeline;

		void createVertexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void createIndexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void createDescriptorHeap(ComPtr<ID3D12Device2> device);
		void createTexture(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void createUniformBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator);
		void createGraphicsPipeline(ComPtr<ID3D12Device2> device);

	public:
		std::vector<Vertex3D> vertices;
		std::vector<uint16_t> indices;
		std::string textureFileName;
		bool enableWireframe = false;

		void createCubeTex(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void cleanUpCubeTex();
		void updateCubeTex(glm::mat4 viewMatrix, int width, int height);
		void recordCubeTex(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv, int width, int height);
		void reloadPipeline(ComPtr<ID3D12Device2> device);
	};
}
#endif