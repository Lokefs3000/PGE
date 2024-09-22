#pragma once

#include "RenderHardware/SwapChain.hpp"
#include "URef.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>

namespace pge
{
	class D3D12SwapChain : public SwapChain
	{
	private:
		URef<IDXGISwapChain4> m_SwapChain;
		URef<ID3D12DescriptorHeap> m_RTVHeap;
		URef<ID3D12Resource> m_RenderTargets[3];

		uint32_t m_BackBufferIndex = 0;
	public:
		D3D12SwapChain(IDXGIFactory4* factory, ID3D12Device12* device, ID3D12CommandQueue* direct, SwapChainCreateInfo& createInfo);
		~D3D12SwapChain();

		void Present();
	};
}