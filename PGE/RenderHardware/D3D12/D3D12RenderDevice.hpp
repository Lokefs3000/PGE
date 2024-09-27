#pragma once

#include "RenderHardware/BaseRenderDevice.hpp"
#include "URef.hpp"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <D3D12MemAlloc.h>
#include <memory>

namespace pge
{
	class D3D12DescriptorHeapManager;

	class D3D12GraphicsDevice : public BaseRenderDevice
	{
	private:
		URef<ID3D12SDKConfiguration1> m_SDKConfig;
		URef<ID3D12DeviceFactory> m_DeviceFactory;

		URef<IDXGIFactory4> m_Factory;
		URef<IDXGIAdapter4> m_Adapter;
		URef<ID3D12Device12> m_Device;

		URef<ID3D12CommandQueue> m_DirectQueue;
		URef<ID3D12CommandQueue> m_ComputeQueue;

		URef<D3D12MA::Allocator> m_Allocator;

		std::unique_ptr<D3D12DescriptorHeapManager> m_DescriptorHeapManager;
	public:
		D3D12GraphicsDevice(RenderDeviceCreateInfo&& createInfo);
		~D3D12GraphicsDevice() override;

		std::unique_ptr<SwapChain> CreateSwapChain(SwapChainCreateInfo&& createInfo) override;
		std::unique_ptr<CommandList> CreateCommandList(CommandListCreateInfo&& createInfo) override;
		std::unique_ptr<DeviceBuffer> CreateDeviceBuffer(DeviceBufferCreateInfo&& createInfo) override;
		std::unique_ptr<GraphicsPipeline> CreateGraphicsPipeline(GraphicsPipelineCreateInfo&& createInfo) override;

		void SubmitCommandList(CommandList* commandList) override;
		void PresentSwapChain(SwapChain* swapChain) override;
	};
}