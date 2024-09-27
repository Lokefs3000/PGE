#pragma once

#include <vector>
#include <map>
#include <mutex>
#include <d3d12.h>

#include "URef.hpp"

namespace pge
{
	class D3D12SignatureContainer
	{
	private:
		URef<ID3D12Device10> m_Device;

		std::mutex m_SignatureMutex;
		std::map<uint8_t, URef<ID3D12RootSignature>> m_SignatureMap;
	public:
		D3D12SignatureContainer(ID3D12Device10* device);
		~D3D12SignatureContainer();

		URef<ID3D12RootSignature> RetrieveSignature(uint8_t cbvCount, uint8_t uavCount);
	};
}