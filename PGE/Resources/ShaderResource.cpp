#include "pch.h"
#include "ShaderResource.hpp"

#include "RenderHardware/BaseRenderDevice.hpp"
#include "RenderHardware/GraphicsPipeline.hpp"

pge::ShaderResource::ShaderResource(std::string name)
{
	m_Name = name;
}

void pge::ShaderResource::SetupThreaded(BaseRenderDevice* device, GraphicsPipelineCreateInfo& createInfo)
{
	m_Pipeline = device->CreateGraphicsPipeline(std::move(createInfo));
}

std::string_view pge::ShaderResource::GetName()
{
	return m_Name;
}
