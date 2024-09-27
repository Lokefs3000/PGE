#include "ODShaderPackage.hpp"

#include "ShaderParser.hpp"

#include <spdlog/spdlog.h>
#include <Utility/FileUtility.hpp>
#include <Types/Stopwatch.hpp>
#include <wrl/client.h>
#include <dxcapi.h>
#include <map>

using namespace Microsoft::WRL;

static ed::ShaderParser s_Parser{};

struct CachedShaderFormat
{
	uint32_t Magic;
	uint16_t Version;

	size_t Timestamp;

	uint16_t VertexBlobSize;
	uint16_t PixelBlobSize;

	static const uint32_t GlobalMagic = 0x43A325;
	static const uint16_t GlobalVersion = 1;
};

static std::map<std::string_view, ed::SemanticId> s_Semantics = {
	{ "POSITION", ed::SemanticId::POSITION },
	{ "TEXCOORD", ed::SemanticId::TEXCOORD },
	{ "NORMAL", ed::SemanticId::NORMAL },
	{ "BITANGENT", ed::SemanticId::BITANGENT },
	{ "TANGENT", ed::SemanticId::TANGENT },
	{ "COLOR", ed::SemanticId::COLOR }
};

void ed::ODShaderPackage::CompileShaderSource(ShaderFileData& data)
{
	auto logger = spdlog::get("ED");
	logger->info("Compiling shader: \"{}\" (\"{}\") for API: {}", data.LocationName, data.SourceLocation.string(), "Direct3D12");

	ShaderParseResult parseRes{};
	{
		std::vector<char> source{};
		if (!pge::FileUtility::ReadFile(data.SourceLocation, source))
		{
			logger->error("Failed to read shader source: \"{}\"", data.SourceLocation.string());
			return;
		}

		if (!s_Parser.ParseShader(source, parseRes))
		{
			logger->error("Failed to parse shader source: \"{}\"", data.SourceLocation.string());
			return;
		}

		data.LocationName = parseRes.Tags.Values["Location"];
	}

	ComPtr<IDxcCompiler3> compiler;
	{
		HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.GetAddressOf()));
		if (FAILED(hr))
		{
			logger->error("Failed to create instance of shader compiler: {}!", hr);
		}
	}

	std::vector<char> vertexBinary;
	std::vector<char> pixelBinary;
	{
		DxcBuffer sourceBuffer{
			.Ptr = parseRes.ResultBuffer.data(),
			.Size = parseRes.ResultBuffer.size()
		};

		{
			std::array<const wchar_t*, 11> arguments = {
				L"-Qstrip_debug",
				L"-Qstrip_reflect",
				L"-O3",
				L"-E",
				L"vertex",
				L"-HV",
				L"2021",
				L"-T",
				L"vs_6_6",
				DXC_ARG_WARNINGS_ARE_ERRORS,
				DXC_ARG_DEBUG,
			};

			ComPtr<IDxcResult> result;
			HRESULT hr = compiler->Compile(&sourceBuffer, arguments.data(), arguments.size(), nullptr, IID_PPV_ARGS(result.GetAddressOf()));

			ComPtr<IDxcBlobEncoding> errorBuffer;
			result->GetErrorBuffer(errorBuffer.GetAddressOf());
			if (errorBuffer != nullptr && errorBuffer->GetBufferSize() > 0)
				logger->error("Failed to compile vertex shader: {}\n{}", hr, (char*)errorBuffer->GetBufferPointer());

			if ((errorBuffer != nullptr && errorBuffer->GetBufferSize() > 0) || FAILED(hr)) return;

			ComPtr<IDxcBlob> binary{};
			if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(binary.GetAddressOf()), nullptr)))
			{
				logger->error("Failed to get vertex shader binary!");
				return;
			}

			vertexBinary.resize(binary->GetBufferSize());
			memcpy(vertexBinary.data(), binary->GetBufferPointer(), vertexBinary.size());
		}
		{
			std::array<const wchar_t*, 11> arguments = {
				L"-Qstrip_debug",
				L"-Qstrip_reflect",
				L"-O3",
				L"-E",
				L"pixel",
				L"-HV",
				L"2021",
				L"-T",
				L"ps_6_6",
				DXC_ARG_WARNINGS_ARE_ERRORS,
				DXC_ARG_DEBUG,
			};

			ComPtr<IDxcResult> result;
			HRESULT hr = compiler->Compile(&sourceBuffer, arguments.data(), arguments.size(), nullptr, IID_PPV_ARGS(result.GetAddressOf()));

			ComPtr<IDxcBlobEncoding> errorBuffer;
			result->GetErrorBuffer(errorBuffer.GetAddressOf());
			if (errorBuffer != nullptr && errorBuffer->GetBufferSize() > 0)
				logger->error("Failed to compile pixel shader: {}\n{}", hr, (char*)errorBuffer->GetBufferPointer());

			if ((errorBuffer != nullptr && errorBuffer->GetBufferSize() > 0) || FAILED(hr)) return;

			ComPtr<IDxcBlob> binary;
			if (FAILED(result->GetResult(binary.GetAddressOf())))
			{
				logger->error("Failed to get pixel shader binary!");
				return;
			}
			size_t t = binary->GetBufferSize();

			pixelBinary.resize(binary->GetBufferSize());
			memcpy(pixelBinary.data(), binary->GetBufferPointer(), pixelBinary.size());
		}
	}

	{
		data.PipelineInfo.InputLayout.resize(parseRes.Inputs.size());

		uint8_t input = 0;
		for (ShaderInputParam& param : parseRes.Inputs)
		{
			pge::InputElementDescription& elem = data.PipelineInfo.InputLayout[input++];
			
			switch (param.Semantic)
			{
			case SemanticId::POSITION: elem.SemanticName = "POSITION"; break;
			case SemanticId::TEXCOORD: elem.SemanticName = "TEXCOORD"; break;
			case SemanticId::NORMAL: elem.SemanticName = "NORMAL"; break;
			case SemanticId::BITANGENT: elem.SemanticName = "BITANGENT"; break;
			case SemanticId::TANGENT: elem.SemanticName = "TANGENT"; break;
			case SemanticId::COLOR: elem.SemanticName = "COLOR"; break;
			case SemanticId::SV_POSITION: elem.SemanticName = "SV_POSITION"; break;
			default: break;
			}
			elem.SemanticIndex = param.Slot;
			elem.Size = param.Size;
		}
	}

	{
		data.PipelineInfo.Rasterizer.FillMode = pge::FillMode::Solid;
		if (parseRes.Tags.Values.contains("RenderFace"))
		{
			std::string& str = parseRes.Tags.Values["RenderFace"];
			if (str == "Back")
				data.PipelineInfo.Rasterizer.CullMode = pge::CullMode::Front;
			else if (str == "Front")
				data.PipelineInfo.Rasterizer.CullMode = pge::CullMode::Back;
			else if (str == "Both")
				data.PipelineInfo.Rasterizer.CullMode = pge::CullMode::None;
		}
	}

	if (parseRes.Tags.Values.contains("RenderMode"))
	{
		std::string& str = parseRes.Tags.Values["RenderMode"];
		if (str == "Solid")
		{
			data.PipelineInfo.Blend.BlendEnable = false;
			data.PipelineInfo.Blend.LogicOpEnable = false;
		}
		else if (str == "Transparent")
		{
			data.PipelineInfo.Blend.BlendEnable = true;
			data.PipelineInfo.Blend.SrcBlend = pge::Blend::SourceAlpha;
			data.PipelineInfo.Blend.DestBlend = pge::Blend::DestinationAlpha;
			data.PipelineInfo.Blend.BlendOp = pge::BlendOperation::Add;
			data.PipelineInfo.Blend.SrcBlendAlpha = pge::Blend::One;
			data.PipelineInfo.Blend.DestBlendAlpha = pge::Blend::Zero;
			data.PipelineInfo.Blend.BlendOpAlpha = pge::BlendOperation::Add;
		}
		else if (str == "Wireframe")
		{
			data.PipelineInfo.Blend.BlendEnable = false;
			data.PipelineInfo.Blend.LogicOpEnable = false;
			data.PipelineInfo.Rasterizer.FillMode = pge::FillMode::Wireframe;
		}
	}

	{
		data.PipelineInfo.PrimitiveTopologyType = pge::PrimitiveTopology::Triangle;
	}

	{
		CachedShaderFormat format{};
		format.Magic = CachedShaderFormat::GlobalMagic;
		format.Version = CachedShaderFormat::GlobalVersion;
		format.Timestamp = std::filesystem::last_write_time(data.SourceLocation).time_since_epoch().count();
		format.VertexBlobSize = vertexBinary.size();
		format.PixelBlobSize = pixelBinary.size();

		size_t hash = std::hash<std::string>()(data.LocationName);
		std::string fileName = fmt::format("{}.{}", hash, (uint8_t)ShaderAPI::Direct3D12);

		std::fstream stream = pge::FileUtility::OpenStream(m_CachePath / fileName, pge::StreamDirection::Out, true);
		if (!stream.is_open() || stream.bad())
		{
			logger->error("Failed to open write stream for cached shader file: \"{}\"! Error: {}", fileName, std::strerror(errno));
			stream.close();
			return;
		}

		stream.write((const char*)&format, sizeof(CachedShaderFormat));
		stream.write((const char*)vertexBinary.data(), vertexBinary.size());
		stream.write((const char*)pixelBinary.data(), pixelBinary.size());

		stream.write((char*)&data.PipelineInfo.Blend, sizeof(pge::RenderTargetBlendDescription));
		stream.write((char*)&data.PipelineInfo.Rasterizer, sizeof(pge::RasterizerDescription));
		stream.write((char*)&data.PipelineInfo.DepthStencil, sizeof(pge::DepthStencilDescription));
		{ uint8_t sz = data.PipelineInfo.InputLayout.size(); stream.write((char*)&sz, sizeof(uint8_t)); }
		for (pge::InputElementDescription& inputs : data.PipelineInfo.InputLayout)
		{
			ed::SemanticId semantic = s_Semantics[inputs.SemanticName];
			stream.write((const char*)&semantic, sizeof(ed::SemanticId));
			stream.write((const char*)&inputs + sizeof(std::string_view), sizeof(uint8_t) * 2);
		}

		stream.write((char*)&data.PipelineInfo.PrimitiveTopologyType, sizeof(pge::PrimitiveTopology));

		{ uint8_t sz = parseRes.CBParams.size(); stream.write((char*)&sz, sizeof(uint8_t)); }
		{ uint8_t sz = parseRes.Params.size(); stream.write((char*)&sz, sizeof(uint8_t)); }

		for (ShaderCBParam& cb : parseRes.CBParams)
		{
			uint8_t nameLength = cb.Name.size();
			stream.write((char*)&nameLength, sizeof(uint8_t));
			stream.write(cb.Name.data(), nameLength);

			stream.write((char*)&cb.Index, sizeof(uint16_t));
			stream.write((char*)&cb.Visibility, sizeof(ShaderVisibilityFlag));
			stream.write((char*)&cb.IsStructured, sizeof(bool));

			{ uint8_t sz = cb.Variables.size(); stream.write((char*)&sz, sizeof(uint8_t)); }
			for (ShaderVariableData param : cb.Variables)
			{
				pge::ParameterSize size;
				switch (param.VariableType)
				{
				case ShaderTokenType::Float : size = pge::ParameterSize::Float1; break;
				case ShaderTokenType::Float2: size = pge::ParameterSize::Float2; break;
				case ShaderTokenType::Float3: size = pge::ParameterSize::Float3; break;
				case ShaderTokenType::Float4: size = pge::ParameterSize::Float4; break;
				case ShaderTokenType::Float4x4: size = pge::ParameterSize::Double4; break;
				case ShaderTokenType::UInt: size = pge::ParameterSize::Float1; break;
				case ShaderTokenType::UInt2: size = pge::ParameterSize::Float2; break;
				case ShaderTokenType::UInt3: size = pge::ParameterSize::Float3; break;
				case ShaderTokenType::UInt4: size = pge::ParameterSize::Float4; break;
				case ShaderTokenType::Byte4: size = pge::ParameterSize::Byte4; break;
				default: break;
				}

				stream.write((char*)&size, sizeof(pge::ParameterSize));
				stream.write((char*)&param.Semantic, sizeof(SemanticId));
				if (param.Semantic != SemanticId::UNKOWN) stream.write((char*)&param.Slot, sizeof(uint8_t));
			}
		}

		for (ShaderValueParam& val : parseRes.Params)
		{
			uint8_t nameLength = val.Name.size();
			stream.write((char*)&nameLength, sizeof(uint8_t));
			stream.write(val.Name.data(), nameLength);

			stream.write((char*)&val.Id, sizeof(ValueId));
			stream.write((char*)&val.Visibility, sizeof(ShaderVisibilityFlag));
			stream.write((char*)&val.Index, sizeof(uint16_t));

			switch (val.Id)
			{
			case ValueId::Texture:
			{
				pge::TextureDimension dim = (pge::TextureDimension)((uint8_t)val.TX.Dimension - (uint8_t)ShaderTokenType::Texture1D);
				stream.write((char*)&dim, sizeof(pge::TextureDimension));
				break;
			}
			case ValueId::SamplerState:
			{
				stream.write((char*)&val.SP.Static, sizeof(bool));
				if (val.SP.Static)
				{
					stream.write((char*)&val.SP.Filter, sizeof(pge::TextureFilter));
					stream.write((char*)&val.SP.AddressU, sizeof(pge::AddressMode));
					stream.write((char*)&val.SP.AddressV, sizeof(pge::AddressMode));
				}
				break;
			}
			default: break;
			}
		}

		stream.close();
	}

	((uint8_t&)data.CacheAPI) |= (uint8_t)ShaderAPI::Direct3D12;
}

std::vector<char> ed::ODShaderPackage::ReadCompiledBinary(ShaderFileData& data, pge::ShaderModule module, bool hasRetried)
{
	auto logger = spdlog::get("ED");

	if (((uint8_t)data.CacheAPI & (uint8_t)ShaderAPI::Direct3D12) == 0)
	{
		logger->error("Shader cannot be loaded because its either not compiled or is for a wrong api! Shader: ", data.LocationName);
		return {};
	}

	size_t hash = std::hash<std::string>()(data.LocationName);
	std::string fileName = fmt::format("{}.{}", hash, (uint8_t)ShaderAPI::Direct3D12);

	std::fstream stream = pge::FileUtility::OpenStream(m_CachePath / fileName, pge::StreamDirection::In, false);
	if (stream.bad() || !stream.is_open())
	{
		logger->error("Failed to open read stream for cached shader file: \"{}\"! Error: {}", fileName, std::strerror(errno));
		stream.close();
		return {};
	}

	CachedShaderFormat shader{};
	stream.read((char*)&shader, sizeof(CachedShaderFormat));

	if (shader.Magic != CachedShaderFormat::GlobalMagic)
	{
		logger->error("Invalid magic for cached shader: {}!", data.LocationName);
		stream.close();

		if (!hasRetried)
		{
			CompileShaderSource(data);
			return ReadCompiledBinary(data, module, true);
		}
	}

	if (shader.Version != CachedShaderFormat::GlobalVersion)
	{
		logger->error("Invalid version for cached shader: {}!", data.LocationName);
		stream.close();

		if (!hasRetried)
		{
			CompileShaderSource(data);
			return ReadCompiledBinary(data, module, true);
		}
	}

	if (shader.Timestamp != std::filesystem::last_write_time(data.SourceLocation).time_since_epoch().count())
	{
		logger->error("Shader timestamp invalid or too old: {}!", data.LocationName);
		stream.close();

		if (!hasRetried)
		{
			CompileShaderSource(data);
			return ReadCompiledBinary(data, module, true);
		}
	}

	std::vector<char> binary{};
	switch (module)
	{
	case pge::ShaderModule::Vertex:
		binary.resize(shader.VertexBlobSize);
		stream.read(binary.data(), binary.size());
		break;
	case pge::ShaderModule::Pixel:
		stream.seekg(shader.VertexBlobSize, std::ios::cur);
		binary.resize(shader.PixelBlobSize);
		stream.read(binary.data(), binary.size());
		break;
	default:
		logger->error("Invalid module for shader binary!");
		stream.close();
		return {};
	}

	stream.close();
	return binary;
}

ed::ODShaderPackage::ODShaderPackage(std::filesystem::path shaders, std::filesystem::path cache)
{
	auto logger = spdlog::get("ED");

	m_CachePath = cache;

	logger->info("Warming up shaders from: \"{}\"..", shaders.string());

	pge::Stopwatch stopwatch(true);
	for (std::filesystem::path loc : std::filesystem::recursive_directory_iterator(shaders))
	{
		if (loc.has_extension())
		{
			std::filesystem::path ext = loc.extension();
			if (ext == ".shader")
			{
				ShaderTags tags{};

				std::vector<char> source{};
				if (!pge::FileUtility::ReadFile(loc, source))
				{
					logger->error("Failed to load shader source: \"{}\"! Tags will not be processed..", loc.string());
					tags.Values.insert(std::make_pair("Location", loc.filename().string().c_str()));
				}
				else if (!s_Parser.ParseTags(source, tags))
				{
					logger->error("Failed to parse shader source: \"{}\"! Tags will not be processed..", loc.string());
					tags.Values.insert(std::make_pair("Location", loc.filename().string().c_str()));
				}

				std::string location = tags.Values["Location"];
				
				ShaderAPI api = ShaderAPI::None;
				{
					size_t hash = std::hash<std::string>()(location);

					//check d3d12
					{
						std::string fileName = fmt::format("{}.{}", hash, (uint8_t)ShaderAPI::Direct3D12);
						if (std::filesystem::exists(m_CachePath / fileName))
							((uint8_t&)api) |= (uint8_t)ShaderAPI::Direct3D12;
					}

					//check vulkan
					{
						std::string fileName = fmt::format("{}.{}", hash, (uint8_t)ShaderAPI::Vulkan);
						if (std::filesystem::exists(m_CachePath / fileName))
							((uint8_t&)api) |= (uint8_t)ShaderAPI::Vulkan;
					}
				}

				m_FileData.push_back(ShaderFileData{
					.LocationName = location,
					.SourceLocation = loc,
					.CacheAPI = api
					});
			}
		}
	}
	stopwatch.Stop();

	logger->info("Warming up shaders took: {}ms", stopwatch.GetDuration());
}

ed::ODShaderPackage::~ODShaderPackage()
{
}

void ed::ODShaderPackage::LowMemory()
{
}

bool ed::ODShaderPackage::ShaderDoesExist(std::string_view name)
{
	for (ShaderFileData& data : m_FileData)
	{
		if (data.LocationName == name)
		{
			return true;
		}
	}

	return false;
}

std::vector<char> ed::ODShaderPackage::RequestShaderBytes(std::string_view shader, pge::ShaderModule module)
{
	for (ShaderFileData& data : m_FileData)
	{
		if (data.LocationName == shader)
		{
			if (((uint8_t)data.CacheAPI & (uint8_t)ShaderAPI::Direct3D12) == 0)
				CompileShaderSource(data);
			return ReadCompiledBinary(data, module);
		}
	}

	spdlog::get("ED")->error("Shader binary requested for unkown shader: \"{}\"!", shader);
	return {};
}

bool ed::ODShaderPackage::LoadGraphicsPipelineCreateInfo(std::string_view name, pge::GraphicsPipelineCreateInfo& createInfo)
{
	auto logger = spdlog::get("ED");

	for (ShaderFileData& data : m_FileData)
	{
		if (data.LocationName == name)
		{
			if (((uint8_t)data.CacheAPI & (uint8_t)ShaderAPI::Direct3D12) == 0)
			{
				logger->error("Shader cannot be loaded because its either not compiled or is for a wrong api! Shader: ", data.LocationName);
				return {};
			}

			size_t hash = std::hash<std::string>()(data.LocationName);
			std::string fileName = fmt::format("{}.{}", hash, (uint8_t)ShaderAPI::Direct3D12);

			std::fstream stream = pge::FileUtility::OpenStream(m_CachePath / fileName, pge::StreamDirection::In, false);
			if (stream.bad() || !stream.is_open())
			{
				logger->error("Failed to open read stream for cached shader file: \"{}\"! Error: {}", fileName, std::strerror(errno));
				stream.close();
				return {};
			}

			CachedShaderFormat shader{};
			stream.read((char*)&shader, sizeof(CachedShaderFormat));

			if (shader.Magic != CachedShaderFormat::GlobalMagic)
			{
				logger->error("Invalid magic for cached shader: {}!", data.LocationName);
				stream.close();
				return false;
			}

			if (shader.Version != CachedShaderFormat::GlobalVersion)
			{
				logger->error("Invalid version for cached shader: {}!", data.LocationName);
				stream.close();
				return false;
			}

			if (shader.Timestamp != std::filesystem::last_write_time(data.SourceLocation).time_since_epoch().count())
			{
				logger->error("Shader timestamp invalid or too old: {}!", data.LocationName);
				stream.close();
				return false;
			}

			stream.seekg(shader.VertexBlobSize + shader.PixelBlobSize + sizeof(CachedShaderFormat));

			stream.read((char*)&createInfo.Blend, sizeof(pge::RenderTargetBlendDescription));
			stream.read((char*)&createInfo.Rasterizer, sizeof(pge::RasterizerDescription));
			stream.read((char*)&createInfo.DepthStencil, sizeof(pge::DepthStencilDescription));

			uint8_t count = 0; stream.read((char*)&count, sizeof(uint8_t));
			createInfo.InputLayout.resize(count);

			for (pge::InputElementDescription& elem : createInfo.InputLayout)
			{
				ed::SemanticId id;
				stream.read((char*)&id, sizeof(ed::SemanticId));
				
				switch (id)
				{
				case ed::SemanticId::POSITION: elem.SemanticName = "POSITION"; break;
				case ed::SemanticId::TEXCOORD: elem.SemanticName = "TEXCOORD"; break;
				case ed::SemanticId::NORMAL: elem.SemanticName = "NORMAL"; break;
				case ed::SemanticId::BITANGENT: elem.SemanticName = "BITANGENT"; break;
				case ed::SemanticId::TANGENT: elem.SemanticName = "TANGENT"; break;
				case ed::SemanticId::COLOR: elem.SemanticName = "COLOR"; break;
				case ed::SemanticId::SV_POSITION:
				case ed::SemanticId::UNKOWN:
				default: break;
				}
				
				stream.read((char*)&elem + sizeof(std::string_view), sizeof(uint8_t) * 2);
			}

			stream.read((char*)&createInfo.PrimitiveTopologyType, sizeof(pge::PrimitiveTopology));

			stream.read((char*)&count, sizeof(uint8_t));
			uint8_t regParams = 0; stream.read((char*)&regParams, sizeof(uint8_t));
			createInfo.Parameters.resize(count + regParams);

			for (size_t i = 0; i < count; i++)
			{
				pge::ShaderValueParameter& param = createInfo.Parameters[i];
				pge::ShaderValueParameter::ConstantBufferData cb{};

				param.Id = pge::ShaderValueId::ConstantBuffer;

				uint8_t nameLength = 0; stream.read((char*)&nameLength, sizeof(uint8_t));
				param.Name.resize(nameLength); stream.read(param.Name.data(), nameLength);

				stream.read((char*)&param.Index, sizeof(uint16_t));
				stream.read((char*)&param.Visbility, sizeof(ShaderVisibilityFlag));
				stream.read((char*)&cb.IsStructured, sizeof(bool));

				uint8_t values = 0; stream.read((char*)&values, sizeof(uint8_t));
				cb.Variables.resize(values);

				for (size_t j = 0; j < values; j++)
				{
					pge::ShaderLocalVariable& variable = cb.Variables[j];
					stream.read((char*)&variable.Size, sizeof(pge::ParameterSize));
					stream.read((char*)&variable.Semantic, sizeof(pge::SemanticId));
					if (variable.Semantic != pge::SemanticId::Unkown) stream.read((char*)&variable.Semantic, sizeof(pge::SemanticId));
				}

				param.Variants = cb;
			}
			for (size_t i = count; i < regParams + count; i++)
			{
				pge::ShaderValueParameter& param = createInfo.Parameters[i];

				uint8_t nameLength = 0; stream.read((char*)&nameLength, sizeof(uint8_t));
				param.Name.resize(nameLength); stream.read(param.Name.data(), nameLength);

				stream.read((char*)&param.Id, sizeof(pge::ShaderValueId));
				stream.read((char*)&param.Visbility, sizeof(pge::ShaderVisibility));
				stream.read((char*)&param.Index, sizeof(uint16_t));

				switch (param.Id)
				{
				case pge::ShaderValueId::Texture:
				{
					pge::ShaderValueParameter::TextureData tx{};

					stream.read((char*)&tx.Dimension, sizeof(pge::TextureDimension));

					param.Variants = tx;
					break;
				}
				case pge::ShaderValueId::SamplerState:
				{
					pge::ShaderValueParameter::SamplerStateData ss{};

					stream.read((char*)&ss.IsStatic, sizeof(bool));
					if (ss.IsStatic)
					{
						stream.read((char*)&ss.Filter, sizeof(pge::TextureFilter));
						stream.read((char*)&ss.AddressU, sizeof(pge::AddressMode));
						stream.read((char*)&ss.AddressV, sizeof(pge::AddressMode));
					}

					param.Variants = ss;
					break;
				}
				default: break;
				}
			}

			return true;
		}
	}

	logger->error("Failed to find shader for pipeline create info: \"{}\"", name);
	return false;
}
