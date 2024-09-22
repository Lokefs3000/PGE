#include "ShaderCompiler.hpp"

#include <d3d12shader.h>
#include <dxcapi.h>

#include <taskflow/taskflow.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

static std::shared_ptr<spdlog::logger> s_Logger;

ed::ShaderCompiler::ShaderCompiler()
{
	s_Logger = spdlog::stdout_color_mt("SHADER");
}

ed::ShaderCompiler::~ShaderCompiler()
{
	s_Logger.reset();
}

void ed::ShaderCompiler::Compile(ShaderCompileOptions&& options)
{
	{
		const char* targetName = "";
		const char* moduleName = "";

		switch (options.Target)
		{
		case ShaderTargets::DXIL: targetName = "DXIL"; break;
		case ShaderTargets::SPIRV: targetName = "SPIRV"; break;
		default: break;
		}

		switch (options.Module)
		{
		case ShaderModule::Vertex: moduleName = "Vertex"; break;
		case ShaderModule::Pixel: moduleName = "Pixel"; break;
		default: break;
		}

		s_Logger->info("Compiling shader: {}.{}", targetName, moduleName);
	}

	IDxcCompiler3* compiler = nullptr;
	HRESULT hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	std::vector<const wchar_t*> args;

	args.push_back(L"-Qstrip_debug");
	args.push_back(L"-Qstrip_reflect");

	args.push_back(DXC_ARG_WARNINGS_ARE_ERRORS);
	args.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);

	args.push_back(L"-T");
	switch (options.Module)
	{
	case ShaderModule::Vertex: args.push_back(L"vs_6_0"); args.push_back(L"-E"); args.push_back(L"vertex"); break;
	case ShaderModule::Pixel: args.push_back(L"ps_6_0"); args.push_back(L"-E"); args.push_back(L"pixel"); break;
	default: break;
	}

	DxcBuffer buffer{
		.Ptr = (void*)options.Input.data(),
		.Size = (size_t)options.Input.size(),
		.Encoding = 0,
	};

	IDxcResult* res;
	hr = compiler->Compile(&buffer, args.data(), args.size(), nullptr, IID_PPV_ARGS(&res));

	IDxcBlobUtf8* err;
	hr = res->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&err), nullptr);
	if (err && err->GetBufferSize() > 0)
	{
		s_Logger->error((char*)err->GetStringPointer());
	}

	IDxcBlob* reflect = nullptr;
	hr = res->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflect), nullptr);

	res->Release();
	compiler->Release();
}

void ed::ShaderCompiler::CompileArgs(char* argv[], int argc)
{
	tf::Executor exec{};
	tf::Taskflow flow{};

	std::vector<char> inputFile{};
	std::string outputFile;
	uint8_t target = 0;

	for (size_t i = 2; i < argc; i++)
	{
		std::string_view arg = argv[i];
		if (arg == "-i")
		{
			std::ifstream stream(argv[++i], std::ios::ate | std::ios::binary);

			inputFile.resize(stream.tellg());
			stream.seekg(0);

			stream.read(inputFile.data(), inputFile.size());

			stream.close();
		}
		else if (arg == "-o")
		{
			outputFile = argv[++i];
		}
		else if (arg.starts_with("-t"))
		{
			std::string_view begin = arg.substr(2);
			if (begin == "dxil")
				target |= (uint8_t)ShaderTargets::DXIL;
			else if (begin == "spirv")
				target |= (uint8_t)ShaderTargets::SPIRV;
		}
	}

	std::vector<char> DXIL_vsBytes;
	std::vector<char> DXIL_psBytes;
	if ((target & (uint8_t)ShaderTargets::DXIL) > 0)
	{
		flow.emplace([&, n = this]() { n->Compile(ShaderCompileOptions{ inputFile, DXIL_vsBytes, ShaderTargets::DXIL, ShaderModule::Vertex }); });
		flow.emplace([&, n = this]() { n->Compile(ShaderCompileOptions{ inputFile, DXIL_psBytes, ShaderTargets::DXIL, ShaderModule::Pixel }); });
	}

	std::vector<char> SPIRV_vsBytes;
	std::vector<char> SPIRV_psBytes;
	if ((target & (uint8_t)ShaderTargets::SPIRV) > 0)
	{
		flow.emplace([&, n = this]() { n->Compile(ShaderCompileOptions{ inputFile, SPIRV_vsBytes, ShaderTargets::SPIRV, ShaderModule::Vertex }); });
		flow.emplace([&, n = this]() { n->Compile(ShaderCompileOptions{ inputFile, SPIRV_psBytes, ShaderTargets::SPIRV, ShaderModule::Pixel }); });
	}

	exec.run(flow);
	exec.wait_for_all();
}
