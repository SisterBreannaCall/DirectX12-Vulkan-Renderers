#ifndef QUEUEDX_H
#define QUEUEDX_H

#include <d3d12.h>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <queue>

namespace DirectX
{
	class QueueDX
	{
	private:
		struct CommandAllocatorEntry
		{
			uint64_t fenceValue;
			ComPtr<ID3D12CommandAllocator> commandAllocator;
		};

		using CommandAllocatorQueue = std::queue<CommandAllocatorEntry>;
		using commandListQueue = std::queue<ComPtr<ID3D12GraphicsCommandList2> >;

		D3D12_COMMAND_LIST_TYPE q_commandListType;
		ComPtr<ID3D12CommandQueue> q_d3d12CommandQueue;
		ComPtr<ID3D12Fence> q_d3d12Fence;
		HANDLE q_fenceEvent;
		uint64_t q_fenceValue;
		CommandAllocatorQueue q_commandAllocatorQueue;
		commandListQueue q_commandListQueue;

	protected:
		ComPtr<ID3D12CommandAllocator> createCommandAllocator(ComPtr<ID3D12Device2> device);
		ComPtr<ID3D12GraphicsCommandList2> createCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> allocator);

	public:
		QueueDX(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type);
		virtual ~QueueDX();

		ComPtr<ID3D12GraphicsCommandList2> getCommandList(ComPtr<ID3D12Device2> device);

		uint64_t executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList);

		uint64_t signal();
		bool isFenceComplete(uint64_t fenceValue);
		void waitForFenceValue(uint64_t fenceValue);
		void flush();

		ComPtr<ID3D12CommandQueue> getD3D12CommandQueue();
	};
}
#endif