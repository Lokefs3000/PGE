#include "pch.h"
#include "ShaderHandler.hpp"

#include <map>
#include <taskflow/taskflow.hpp>

#include "ShaderPackage.hpp"
#include "Resources/ShaderResource.hpp"
#include "RenderHardware/GraphicsPipeline.hpp"

void pge::ShaderHandler::LoadAndPopShader()
{
	ShaderQueueElement elem = m_ShaderQueue.front();
	m_ShaderQueue.pop();

	std::string_view name = elem.Resource->GetName();

	GraphicsPipelineCreateInfo createInfo{};
	createInfo.VertexShader = elem.Package->RequestShaderBytes(name, ShaderModule::Vertex);
	createInfo.PixelShader = elem.Package->RequestShaderBytes(name, ShaderModule::Pixel);
	elem.Package->LoadGraphicsPipelineCreateInfo(name, createInfo);

	elem.Resource->SetupThreaded(m_RenderDevice, createInfo);
}

pge::ShaderHandler::ShaderHandler()
{
}

pge::ShaderHandler::~ShaderHandler()
{
}

void pge::ShaderHandler::LowMemory()
{
	for (std::unique_ptr<ShaderPackage>& package : m_Packages)
	{
		package->LowMemory();
	}
}

void pge::ShaderHandler::FlushShaders()
{
	for (size_t i = 0; i < m_Resources.size(); i++)
	{
		if (m_Resources[i].second.RefCount() <= 1)
		{
			m_Resources.erase(m_Resources.begin() + i);
		}
	}
}

void pge::ShaderHandler::AddPackage(std::unique_ptr<ShaderPackage>& package)
{
	m_Packages.push_back(std::move(package));
}

pge::ResourcePtr<pge::ShaderResource> pge::ShaderHandler::LoadResource(std::string_view name)
{
	size_t hash = std::hash<std::string_view>()(name);
	for (auto& pair : m_Resources)
	{
		if (pair.first == hash)
			return pair.second;
	}

	for (std::unique_ptr<ShaderPackage>& package : m_Packages)
	{
		if (package->ShaderDoesExist(name))
		{
			m_Resources.push_back(std::make_pair(hash, ResourcePtr<ShaderResource>(new ShaderResource(name.data()))));

			ResourcePtr<ShaderResource> lck = m_Resources.back().second;
			m_ShaderQueue.push(ShaderQueueElement{
				.Package = package.get(),
				.Resource = lck
				});
			m_Executor->async("shader load", std::bind(&ShaderHandler::LoadAndPopShader, this));

			return m_Resources.back().second;
		}
	}

	return nullptr;
}

void pge::ShaderHandler::Connect(BaseRenderDevice* rd, tf::Executor* ex)
{
	m_RenderDevice = rd;
	m_Executor = ex;
}
