#pragma once

#include <filesystem>
#include <toml++/toml.hpp>
#include <mutex>
#include <vector>
#include <crossguid/guid.hpp>

#include <Graphics/Shaders/ShaderPackage.hpp>
#include <RenderHardware/GraphicsPipeline.hpp>

namespace ed
{
	enum class ShaderAPI : uint8_t
	{
		None = 0b00000000,
		Direct3D12 = 0b00000001,
		Vulkan = 0b00000010
	};

	struct ShaderFileData
	{
		std::string LocationName;
		std::filesystem::path SourceLocation;
		ShaderAPI CacheAPI;

		pge::GraphicsPipelineCreateInfo PipelineInfo;
	};

	//"On-Demand" Shader Package
	class ODShaderPackage : public pge::ShaderPackage
	{
	private:
		std::filesystem::path m_CachePath;

		std::vector<ShaderFileData> m_FileData; //accesed multithreaded

		void CompileShaderSource(ShaderFileData& data);
		std::vector<char> ReadCompiledBinary(ShaderFileData& data, pge::ShaderModule module, bool hasRetried = false);
	public:
		ODShaderPackage(std::filesystem::path shaders, std::filesystem::path cache);
		~ODShaderPackage() override;

		void LowMemory() override;

		bool ShaderDoesExist(std::string_view name) override;
		std::vector<char> RequestShaderBytes(std::string_view shader, pge::ShaderModule module) override;
		bool LoadGraphicsPipelineCreateInfo(std::string_view name, pge::GraphicsPipelineCreateInfo& createInfo) override;
	};
}