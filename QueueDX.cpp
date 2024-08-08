#include "QueueDX.h"

namespace DirectX
{
	QueueDX::QueueDX(ComPtr<ID3D12Device2> device, D3D12_COMMAND_LIST_TYPE type)
	{
		q_fenceValue = 0;
		q_commandListType = type;

		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		device->CreateCommandQueue(&desc, IID_PPV_ARGS(&q_d3d12CommandQueue));
		device->CreateFence(q_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&q_d3d12Fence));

		q_fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	}

	QueueDX::~QueueDX()
	{

	}

	uint64_t QueueDX::signal()
	{
		uint64_t fenceValue = ++q_fenceValue;
		q_d3d12CommandQueue->Signal(q_d3d12Fence.Get(), fenceValue);
		return fenceValue;
	}

	bool QueueDX::isFenceComplete(uint64_t fenceValue)
	{
		return q_d3d12Fence->GetCompletedValue() >= fenceValue;
	}

	void QueueDX::waitForFenceValue(uint64_t fenceValue)
	{
		if (!isFenceComplete(fenceValue))
		{
			q_d3d12Fence->SetEventOnCompletion(fenceValue, q_fenceEvent);
			::WaitForSingleObject(q_fenceEvent, DWORD_MAX);
		}
	}

	void QueueDX::flush()
	{
		waitForFenceValue(signal());
	}

	ComPtr<ID3D12CommandAllocator> QueueDX::createCommandAllocator(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		device->CreateCommandAllocator(q_commandListType, IID_PPV_ARGS(&commandAllocator));

		return commandAllocator;
	}

	ComPtr<ID3D12GraphicsCommandList2> QueueDX::createCommandList(ComPtr<ID3D12Device2> device, ComPtr<ID3D12CommandAllocator> allocator)
	{
		ComPtr<ID3D12GraphicsCommandList2> commandList;
		device->CreateCommandList(0, q_commandListType, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

		return commandList;
	}

	ComPtr<ID3D12GraphicsCommandList2> QueueDX::getCommandList(ComPtr<ID3D12Device2> device)
	{
		ComPtr<ID3D12CommandAllocator> commandAllocator;
		ComPtr<ID3D12GraphicsCommandList2> commandList;

		if (!q_commandAllocatorQueue.empty() && isFenceComplete(q_commandAllocatorQueue.front().fenceValue))
		{
			commandAllocator = q_commandAllocatorQueue.front().commandAllocator;
			q_commandAllocatorQueue.pop();

			commandAllocator->Reset();
		}

		else
		{
			commandAllocator = createCommandAllocator(device);
		}

		if (!q_commandListQueue.empty())
		{
			commandList = q_commandListQueue.front();
			q_commandListQueue.pop();
			commandList->Reset(commandAllocator.Get(), nullptr);
		}
		else
		{
			commandList = createCommandList(device, commandAllocator);
		}

		commandList->SetPrivateDataInterface(__uuidof(ID3D12CommandAllocator), commandAllocator.Get());

		return commandList;
	}

	uint64_t QueueDX::executeCommandList(ComPtr<ID3D12GraphicsCommandList2> commandList)
	{
		commandList->Close();

		ID3D12CommandAllocator* commandAllocator;
		UINT dataSize = sizeof(commandAllocator);
		commandList->GetPrivateData(__uuidof(ID3D12CommandAllocator), &dataSize, &commandAllocator);

		ID3D12CommandList* const ppCommandLists[] = {
			commandList.Get()
		};

		q_d3d12CommandQueue->ExecuteCommandLists(1, ppCommandLists);
		uint64_t fenceValue = signal();

		q_commandAllocatorQueue.emplace(CommandAllocatorEntry{ fenceValue, commandAllocator });
		q_commandListQueue.push(commandList);

		commandAllocator->Release();

		return fenceValue;
	}

	ComPtr<ID3D12CommandQueue> QueueDX::getD3D12CommandQueue()
	{
		return q_d3d12CommandQueue;
	}
}