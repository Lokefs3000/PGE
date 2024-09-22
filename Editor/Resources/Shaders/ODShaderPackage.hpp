#pragma once

#include <filesystem>
#include <toml++/toml.hpp>
#include <mutex>
#include <vector>

#include <Graphics/Shaders/ShaderPackage.hpp>

namespace ed
{
	//"On-Demand" Shader Package
	class ODShaderPackage : public pge::ShaderPackage
	{
	private:

	public:
		ODShaderPackage(std::filesystem::path shaders, std::filesystem::path cache);
		~ODShaderPackage() override;

		void LowMemory() override;

		bool ShaderDoesExist(std::string_view name) override;
		std::vector<char> RequestShaderBytes(std::string_view shader, pge::ShaderModule module) override;
		bool LoadGraphicsPipelineCreateInfo(std::string_view name, pge::GraphicsPipelineCreateInfo& createInfo) override;
	};
}