#pragma once

#include <vector>
#include <string_view>

namespace pge
{
	enum class ShaderModule
	{
		Vertex,
		Pixel
	};

	struct GraphicsPipelineCreateInfo;

	class ShaderPackage
	{
	public:
		ShaderPackage() = default;
		virtual ~ShaderPackage() = default;

		virtual void LowMemory() = 0;

		virtual bool ShaderDoesExist(std::string_view name) = 0;										 //  false = no/err
		virtual std::vector<char> RequestShaderBytes(std::string_view shader, ShaderModule module) = 0;  //  empty = error
		virtual bool LoadGraphicsPipelineCreateInfo(std::string_view name, GraphicsPipelineCreateInfo& createInfo) = 0;		 //  false = error
	};
}