#pragma once

#include <string>
#include <vector>

namespace cfg
{
	enum class ElementType : uint8_t
	{
		Int8,
		Int16,
		Int32,

		Uint8,
		Uint16,
		Uint32,

		Float,
		Double,

		String
	};

	struct ConfigElement
	{
		const char* Name;
		ElementType Type;
		bool IsVector;
	};

	void LoadConfigFromFile(std::string_view file, void* ptr, std::vector<ConfigElement> layout);
	void LoadConfigFromMemory(std::vector<char> buffer, void* ptr, std::vector<ConfigElement> layout);
}