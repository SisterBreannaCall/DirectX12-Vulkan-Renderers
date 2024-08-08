#ifndef LIGHTCUBEDX_H
#define LIGHTCUBEDX_H

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

#include <vector>
#include <array>
#include <algorithm>

namespace DirectX
{
	class LightCubeDX
	{
		struct Vertex3D
		{
			glm::vec3 position;
		};

		struct UniformData
		{
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 projection;
			float padding[16];
		};

	private:
		std::vector<Vertex3D> vertices;
		bool enableWireframe = false;
		ComPtr<ID3D12Resource> vertexBuffer;
		D3D12MA::Allocation* vertexAllocation = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
		UINT vertexCount = 0;
		ComPtr<ID3D12Resource> uniformBuffer;
		D3D12MA::Allocation* uniformAllocation = nullptr;
		ComPtr<ID3D12DescriptorHeap> uniformDescriptorHeap;
		ComPtr<ID3D12RootSignature> rootSignature;
		ComPtr<ID3D12PipelineState> graphicsPipeline;

		void createVertexBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue);
		void createDescriptorHeap(ComPtr<ID3D12Device2> device);
		void createUniformBuffer(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator);
		void createGraphicsPipeline(ComPtr<ID3D12Device2> device, UINT msaaSamples);

	public:
		LPCWSTR vertFileName;
		LPCWSTR fragFileName;
		UniformData uniformData;
		glm::vec4 lightColor;
		glm::vec4 lightPos;

		void createCube(ComPtr<ID3D12Device2> device, D3D12MA::Allocator* allocator, std::shared_ptr<QueueDX> queue, UINT msaaSamples);
		void updateCube(glm::mat4 viewMatrix, int width, int height);
		void recordCube(ComPtr<ID3D12GraphicsCommandList2> commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtv, D3D12_CPU_DESCRIPTOR_HANDLE dsv, int width, int height);
		void cleanUpCube();
		void reloadPipeline(ComPtr<ID3D12Device2> device, UINT msaaSamples);
	};
}
#endif