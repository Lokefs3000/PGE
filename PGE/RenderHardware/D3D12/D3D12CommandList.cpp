#include "pch.h"
#include "D3D12CommandList.hpp"

#include "D3D12Utility.hpp"

#include "D3D12DeviceBuffer.hpp"

pge::D3D12CommandList::D3D12CommandList(ID3D12Device12* device, CommandListCreateInfo& createInfo)
{
	m_MaxFrames = createInfo.MaxFramesInFlight;
	m_CurrentFrame = 1;

	{
		for (size_t i = 0; i < createInfo.MaxFramesInFlight; i++)
		{
			HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_CommandAllocators[i].GetAddressOf()));
			if (FAILED(hr))
			{
				m_CommandAllocators[0].Reset();
				m_CommandAllocators[1].Reset();
				m_CommandAllocators[2].Reset();

				D3D12Utility::PrettyPrint(hr, "Failed to create command allocator!");
				return;
			}
		}
	}

	{
		HRESULT hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocators[0].Get(), nullptr, IID_PPV_ARGS(m_CommandList.GetAddressOf()));
		if (FAILED(hr))
		{
			m_CommandAllocators[0].Reset();
			m_CommandAllocators[1].Reset();
			m_CommandAllocators[2].Reset();

			D3D12Utility::PrettyPrint(hr, "Failed to create command list!");
			return;
		}
	}
}

pge::D3D12CommandList::~D3D12CommandList()
{
	if (m_HasBegun)
		m_CommandList->Close();
}

void pge::D3D12CommandList::Begin()
{
	if (m_HasBegun) return;

	m_CommandAllocators[m_CurrentFrame]->Reset();
	m_CommandList->Reset(m_CommandAllocators[m_CurrentFrame].Get(), nullptr);

	m_HasBegun = true;
}

void pge::D3D12CommandList::Close()
{
	if (!m_HasBegun) return;

	m_CommandList->Close();
	
	m_CurrentFrame++;
	m_CurrentFrame *= m_CurrentFrame < m_MaxFrames;

	m_HasBegun = false;
}

void pge::D3D12CommandList::Submit(ID3D12CommandQueue* commandQueue)
{
	if (!m_HasBegun)
	{
		commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)m_CommandList.GetAddressOf());
	}
}

void pge::D3D12CommandList::SetGraphicsPipeline(GraphicsPipeline* pipeline)
{
}

void pge::D3D12CommandList::SetVertexBuffer(uint8_t slot, DeviceBuffer* buffer)
{
	D3D12_VERTEX_BUFFER_VIEW view{
		.BufferLocation = ((D3D12DeviceBuffer*)buffer)->GetGpuAddress(),
		.SizeInBytes = (uint32_t)buffer->GetByteSize(),
		.StrideInBytes = buffer->GetStride()
	};
	m_CommandList->IASetVertexBuffers(slot, 1, &view);
}

void pge::D3D12CommandList::SetIndexBuffer(DeviceBuffer* buffer)
{
	D3D12_INDEX_BUFFER_VIEW view{
		.BufferLocation = ((D3D12DeviceBuffer*)buffer)->GetGpuAddress(),
		.SizeInBytes = (uint32_t)buffer->GetByteSize()
	};
	
	switch (buffer->GetStride())
	{
	case 2: view.Format = DXGI_FORMAT_R16_UINT;
	case 4: view.Format = DXGI_FORMAT_R32_UINT;
	default: break;
	}

	m_CommandList->IASetIndexBuffer(&view);
}

void pge::D3D12CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t indexOffset, uint32_t vertexOffset)
{
	m_CommandList->DrawIndexedInstanced(indexCount, instanceCount, indexOffset, vertexOffset, 0);
}
