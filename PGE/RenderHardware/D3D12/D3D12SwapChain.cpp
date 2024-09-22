#include "pch.h"
#include "D3D12SwapChain.hpp"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include "D3D12Utility.hpp"

#include <d3dx12.h>

pge::D3D12SwapChain::D3D12SwapChain(IDXGIFactory4* factory, ID3D12Device12* device, ID3D12CommandQueue* direct, SwapChainCreateInfo& createInfo)
{
	{
		DXGI_SWAP_CHAIN_DESC1 desc{};
		desc.Width = createInfo.Width;
		desc.Height = createInfo.Height;
		desc.Format = createInfo.sRGB ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.SampleDesc = { 1, 0 };
		desc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
		desc.BufferCount = createInfo.ImageCount;
		desc.Scaling = DXGI_SCALING_NONE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		URef<IDXGISwapChain1> swapChain{};
		HRESULT hr = factory->CreateSwapChainForHwnd(direct, glfwGetWin32Window(createInfo.Window), &desc, nullptr, nullptr, swapChain.GetAddressOf());
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to create swapchain!");
			return;
		}

		hr = swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf()));
		if (FAILED(hr))
		{
			D3D12Utility::PrettyPrint(hr, "Failed to query newer swapchain!");
			return;
		}

		factory->MakeWindowAssociation(glfwGetWin32Window(createInfo.Window), DXGI_MWA_NO_WINDOW_CHANGES);
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = createInfo.ImageCount;

		HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_RTVHeap.GetAddressOf()));
		if (FAILED(hr))
		{
			m_SwapChain.Reset();

			D3D12Utility::PrettyPrint(hr, "Failed to create render target heap!");
			return;
		}
	}

	{
		uint32_t increment = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_RTVHeap->GetCPUDescriptorHandleForHeapStart();

		for (uint32_t i = 0; i < createInfo.ImageCount; i++)
		{
			HRESULT hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_RenderTargets[i].GetAddressOf()));
			if (FAILED(hr))
			{
				m_RenderTargets[0].Reset();
				m_RenderTargets[1].Reset();
				m_RenderTargets[2].Reset();
				m_RTVHeap.Reset();
				m_SwapChain.Reset();

				D3D12Utility::PrettyPrint(hr, "Failed to acquire buffer of swapchain!");
				return;
			}

			device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, cpuHandle);
			cpuHandle.ptr += i * increment;
		}
	}

	m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

pge::D3D12SwapChain::~D3D12SwapChain()
{
}

void pge::D3D12SwapChain::Present()
{
	DXGI_PRESENT_PARAMETERS params{};
	m_SwapChain->Present1(0, 0, &params);

	m_BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}
