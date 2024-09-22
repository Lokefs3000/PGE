#pragma once

#include <memory>
#include <string>

#include "BasicResource.hpp"

namespace pge
{
	class GraphicsPipeline;

	class BaseRenderDevice;
	struct GraphicsPipelineCreateInfo;

	//TODO: Dynamic load shader pipeline for individual formats (RTV&DSV)
	class ShaderResource : public BasicResource
	{
	private:
		std::unique_ptr<GraphicsPipeline> m_Pipeline;
		std::string m_Name;
	public:
		ShaderResource(std::string name);

		void SetupThreaded(BaseRenderDevice* device, GraphicsPipelineCreateInfo& createInfo);

		std::string_view GetName();
	};
}