#include "pch.h"
#include "D3D12SignatureContainer.hpp"

pge::D3D12SignatureContainer::D3D12SignatureContainer(ID3D12Device10* device)
{
    m_Device = device;
}

pge::D3D12SignatureContainer::~D3D12SignatureContainer()
{
}

pge::URef<ID3D12RootSignature> pge::D3D12SignatureContainer::RetrieveSignature(uint8_t cbvCount, uint8_t uavCount)
{
    std::unique_lock lock(m_SignatureMutex);

    uint8_t key = (cbvCount) | ((uint16_t)uavCount << 4);
    if (m_SignatureMap.contains(key))
        return m_SignatureMap[key];

    D3D12_ROOT_SIGNATURE_DESC desc{};

    return URef<ID3D12RootSignature>();
}
