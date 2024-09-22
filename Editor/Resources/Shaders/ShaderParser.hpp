#pragma once

#include <vector>
#include <map>

#include "Types/SmallString.hpp"
#include "RenderHardware/Common.hpp"

namespace ed
{
	enum class ShaderTokenType : uint8_t
	{
		ConstantBuffer,
		StructuredBuffer,
		SamplerState,
		StaticSamplerState,
		Texture1D,
		Texture2D,
		Texture3D,
		Texture1DArray,
		Texture2DArray,
		Texture3DArray,

		Float,
		Float2,
		Float3,
		Float4,

		Float4x4,

		POSITION,
		COLOR,
		TEXCOORD,
		NORMAL,
		BITANGENT,
		TANGENT,
		SV_POSITON,
		SV_TARGET,

		UInt,
		UInt2,
		UInt3,
		UInt4,

		Struct,
		Return,

		Identifier,
		String,
		Number,
		Integral,

		Colon,
		Semicolon,
		Period, // ,
		Comma,	// .
		Underscore,
		Lessthan,
		Greaterthan,
		Plus,
		Minus,
		Star,
		Slash,
		Equals,

		OpenSquareBracket,
		CloseSquareBracket,
		OpenBracket,
		CloseBracket,
		OpenCurlyBracket,
		CloseCurlyBracket,
	};

	struct ShaderToken
	{
		ShaderTokenType Type;
		union
		{
			float FloatValue;
			uint32_t UIntValue;
		};
		pge::SmallString Snippet;
		uint16_t Line;
	};

	enum class SemanticId : uint8_t
	{
		UNKOWN,
		POSITION,
		TEXCOORD,
		NORMAL,
		BITANGENT,
		TANGENT,
		COLOR,
		SV_POSITION
	};

	struct ShaderInputParam
	{
		SemanticId Semantic;
		uint8_t Slot;
	};

	enum class ValueId
	{
		ConstantBuffer,
		Texture,
		SamplerState
	};

	struct ShaderVariableData
	{
		pge::SmallString Name;
		ShaderTokenType VariableType;

		SemanticId Semantic; //optional
		uint8_t Slot;		 //optional
	};

	enum class ShaderVisibilityFlag : uint8_t
	{
		All,
		Vertex,
		Pixel
	};

	struct ShaderCBParam //destructor bs
	{
		pge::SmallString Name;
		pge::SmallString ValueType;
		uint16_t Index;
		ShaderVisibilityFlag Visibility;
		bool IsStructured;
		std::vector<ShaderVariableData> Variables;
	};

	struct ShaderValueParam
	{
		struct TXData
		{
			ShaderTokenType Dimension;
		};

		struct SPData
		{
			bool Static;

			pge::TextureFilter Filter;
			pge::AddressMode AddressU;
			pge::AddressMode AddressV;
		};

		ValueId Id;
		pge::SmallString Name;
		ShaderVisibilityFlag Visibility;
		uint16_t Index;
		union
		{
			TXData TX; //Texture
			SPData SP; //SamplerState
		};
	};

	struct ShaderTags
	{
		std::vector<std::pair<pge::SmallString, pge::SmallString>> Values;
	};

	struct ShaderSourceBounds
	{
		uint32_t Start;
		uint32_t End;
	};

	struct ShaderTreeNode
	{
		pge::SmallString Name;
		pge::SmallString Type;

		std::map<std::string_view, pge::SmallString> Attributes;
		std::vector<ShaderTreeNode> Children; //gotta figure out a more effecient solution!
	};

	struct ShaderFunctionDefinition
	{
		uint32_t LocalStart;
		uint32_t Start;
		uint32_t End;
	};

	struct ShaderParseResult
	{
		ShaderTags Tags;
		ShaderSourceBounds Bounds;
		std::vector<char> ResultBuffer;
		std::vector<ShaderInputParam> Inputs;
		std::vector<ShaderValueParam> Params;
		std::vector<ShaderCBParam> CBParams;
		ShaderTreeNode AST;
	};

	class ShaderParser
	{
	private:
		struct LocalState
		{
			ShaderParseResult& Result;

			std::vector<char>& Source;
			std::vector<ShaderToken> Tokens;

			uint32_t Start;
			uint32_t Current;

			bool ErrorRaised;

			uint16_t Scan_Line;

			ShaderTreeNode* AST_Active; //oh-noooo its unsafe!!11!1!!! blah blah
		};

#pragma region Scanner
		void InsertTokenAt(LocalState& state, ShaderTokenType type);
		void InsertTokenAt(LocalState& state, ShaderTokenType type, std::string_view snippet);
		void InsertTokenAt(LocalState& state, ShaderTokenType type, float v);
		void InsertTokenAt(LocalState& state, ShaderTokenType type, uint32_t v);

		void Identifier(LocalState& state);
		void Number(LocalState& state);
		void String(LocalState& state, char delim);
#pragma endregion
#pragma region Reflector
		pge::TextureFilter StringConvertFilter(std::string_view string) noexcept;
		pge::AddressMode StringConvertAddress(std::string_view string) noexcept;

		bool ExamineAllStructs(LocalState& state, std::vector<std::pair<pge::SmallString, uint32_t>>& indices);
#pragma endregion
#pragma region AST
		bool ASTIsFunction(LocalState&, std::string_view name);

		void ASTStruct(LocalState& state);
		void ASTConstantBuffer(LocalState& state);
		void ASTSamplerState(LocalState& state);
		void ASTStaticSamplerState(LocalState& state);
		void ASTTexture(LocalState& state, ShaderToken& token);
		void ASTFunction(LocalState& state, ShaderToken& valueType);
		void ASTCall(LocalState& state, ShaderToken& function);
		void ASTVariable(LocalState& state, ShaderToken& valueType);
#pragma endregion
#pragma region Build
		bool ExamineAllFunctions(LocalState& state, std::vector<ShaderFunctionDefinition>& definitions);
#pragma endregion

		void PrintDebug(LocalState& state, ShaderTreeNode& node, uint32_t depth);

		bool PerformScan(LocalState& state);
		bool PerformBounds(LocalState& state);
		bool PerformReflect(LocalState& state);
		bool PerformAST(LocalState& state);
		bool PerformBuild(LocalState& state);
	public:
		bool ParseShader(std::vector<char>& source, ShaderParseResult& result);
	};
}