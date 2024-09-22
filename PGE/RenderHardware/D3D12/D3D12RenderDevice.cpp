#include "pch.h"
#include "D3D12RenderDevice.hpp"

#include <d3d12sdklayers.h>

#define M_D3D12SDKVersion 614
#define M_D3D12SDKPath ".\\D3D12\\"

//IMPORTANT:
//	1. must be exported from the exe and since this is a static library it will be
//	2. must be updated to reflect the current sdk version as this is loader config
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = M_D3D12SDKVersion; }
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = M_D3D12SDKPath; }

#include "D3D12Utility.hpp"

#include "D3D12SwapChain.hpp"
#include "D3D12CommandList.hpp"
#include "D3D12DeviceBuffer.hpp"
#include "D3D12GraphicsPipeline.hpp"

pge::D3D12GraphicsDevice::D3D12GraphicsDevice(RenderDeviceCreateInfo&& createInfo)
{
	{
		HRESULT hr = D3D12GetInterface(CLSID_D3D12SDKConfiguration, IID_PPV_ARGS(m_SDKConfig.GetAddressOf()));
		if (SUCCEEDED(hr))
		{
			//m_SDKConfig->SetSDKVersion(M_D3D12SDKVersion, M_D3D12SDKPath);
		}
		else
		{
			D3D12Utility::PrettyPrint(hr, "Failed to get sdk configuration interface!");
			exit((int)hr);
		}

		m_SDKConfig->CreateDeviceFactory(M_D3D12SDKVersion, M_D3D12SDKPath, IID_PPV_ARGS(m_DeviceFactory.GetAddressOf()));
	}

	{
		URef<ID3D12Debug> debug;
		if (SUCCEEDED(m_DeviceFactory->GetConfigurationInterface(CLSID_D3D12Debug, IID_PPV_ARGS(debug.GetAddressOf()))))
		{
			debug->EnableDebugLayer();
		}
	}

	{
		HRESULT hr = CreateDXGIFactory2(createInfo.DebuggingEnabled * DXGI_CREATE_FACTORY_DEBUG/*big smart*/, IID_PPV_ARGS(m_Factory.GetAddressOf()));
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create DXGI factory!");
			exit((int)hr);
		}
	}

	{
		uint32_t adapterId = 0;
		URef<IDXGIAdapter1> adapter1;

		std::multimap<size_t, URef<IDXGIAdapter4>> adapters{};
		while (SUCCEEDED(m_Factory->EnumAdapters1(adapterId++, adapter1.GetAddressOf())))
		{
			URef<IDXGIAdapter4> adapter4;
			if (SUCCEEDED(adapter1->QueryInterface(IID_PPV_ARGS(adapter4.GetAddressOf()))))
			{
				DXGI_ADAPTER_DESC3 desc{};
				HRESULT hr = adapter4->GetDesc3(&desc);

				size_t score = 0;
				score += desc.DedicatedVideoMemory;

				adapters.insert(std::make_pair(score, adapter4));
			}
		}

		if (!adapters.empty())
		{
			m_Adapter = adapters.rbegin()->second;
		}
		else
		{
			D3D12Utility::PrettyPrint(0, "Failed enumerate adapters!");
			exit(0);
		}
	}

	{
		HRESULT hr = m_DeviceFactory->CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(m_Device.GetAddressOf()));
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create device!");
			exit((int)hr);
		}
	}

	{
		D3D12_COMMAND_QUEUE_DESC desc{};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		HRESULT hr = m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_DirectQueue.GetAddressOf()));
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create a direct command queue!");
			exit((int)hr);
		}

		desc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		        hr = m_Device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_ComputeQueue.GetAddressOf()));
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create a compute command queue!");
			exit((int)hr);
		}
	}

	{
		D3D12MA::ALLOCATOR_DESC desc{};
		desc.Flags = D3D12MA::ALLOCATOR_FLAG_NONE;
		desc.pDevice = m_Device.Get();
		desc.pAdapter = m_Adapter.Get();

		HRESULT hr = D3D12MA::CreateAllocator(&desc, m_Allocator.GetAddressOf());
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create allocator!");
			exit((int)hr);
		}
	}
}

pge::D3D12GraphicsDevice::~D3D12GraphicsDevice()
{
	m_SDKConfig->FreeUnusedSDKs();
}

std::unique_ptr<pge::SwapChain> pge::D3D12GraphicsDevice::CreateSwapChain(SwapChainCreateInfo&& createInfo)
{
	return std::make_unique<D3D12SwapChain>(m_Factory.Get(), m_Device.Get(), m_DirectQueue.Get(), createInfo);
}

std::unique_ptr<pge::CommandList> pge::D3D12GraphicsDevice::CreateCommandList(CommandListCreateInfo&& createInfo)
{
	return std::make_unique<D3D12CommandList>(m_Device.Get(), createInfo);
}

std::unique_ptr<pge::DeviceBuffer> pge::D3D12GraphicsDevice::CreateDeviceBuffer(DeviceBufferCreateInfo&& createInfo)
{
	return std::make_unique<D3D12DeviceBuffer>(m_Device.Get(), m_Allocator.Get(), createInfo);
}

std::unique_ptr<pge::GraphicsPipeline> pge::D3D12GraphicsDevice::CreateGraphicsPipeline(GraphicsPipelineCreateInfo&& createInfo)
{
	return std::make_unique<D3D12GraphicsPipeline>(m_Device.Get(), createInfo);
}

void pge::D3D12GraphicsDevice::SubmitCommandList(CommandList* commandList)
{
	((D3D12CommandList*)commandList)->Submit(m_DirectQueue.Get());
}

void pge::D3D12GraphicsDevice::PresentSwapChain(SwapChain* swapChain)
{
	((D3D12SwapChain*)swapChain)->Present();
}
