#pragma once

#include <vector>

struct IDxcCompiler3;

namespace ed
{
	enum class ShaderTargets : uint8_t
	{
		DXIL = 1,
		SPIRV = 2
	};

	enum class ShaderModule : uint8_t
	{
		Vertex,
		Pixel,
	};

	struct ShaderCompileOptions
	{
		std::vector<char> Input;
		std::vector<char>& Output;

		ShaderTargets Target;
		ShaderModule Module;
	};

	class ShaderCompiler
	{
	private:
		IDxcCompiler3* m_Compiler = nullptr;
	public:
		ShaderCompiler();
		~ShaderCompiler();

		void Compile(ShaderCompileOptions&& options);
		void CompileArgs(char* argv[], int argc);
	};
}