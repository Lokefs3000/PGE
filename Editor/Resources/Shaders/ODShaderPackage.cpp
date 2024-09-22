#include "ODShaderPackage.hpp"

#include "ShaderParser.hpp"

static ed::ShaderParser s_Parser{};

ed::ODShaderPackage::ODShaderPackage(std::filesystem::path shaders, std::filesystem::path cache)
{
}

ed::ODShaderPackage::~ODShaderPackage()
{
}

void ed::ODShaderPackage::LowMemory()
{
}

bool ed::ODShaderPackage::ShaderDoesExist(std::string_view name)
{
	return false;
}

std::vector<char> ed::ODShaderPackage::RequestShaderBytes(std::string_view shader, pge::ShaderModule module)
{
	return std::vector<char>();
}

bool ed::ODShaderPackage::LoadGraphicsPipelineCreateInfo(std::string_view name, pge::GraphicsPipelineCreateInfo& createInfo)
{
	return false;
}
