#include "configuration.hpp"

#include <fstream>

static uint8_t s_MemorySizes[] = {
	sizeof(int8_t),
	sizeof(int16_t),
	sizeof(int32_t),

	sizeof(uint8_t),
	sizeof(uint16_t),
	sizeof(uint32_t),

	sizeof(float),
	sizeof(double),

	sizeof(std::string)
};

void cfg::LoadConfigFromFile(std::string_view file, void* ptr, std::vector<ConfigElement> layout)
{
	std::ifstream stream(file.data(), std::ios::ate | std::ios::binary);
	if (!stream.is_open()) return;

	std::vector<char> buffer;
	buffer.resize(stream.tellg());

	stream.seekg(0);
	stream.read(buffer.data(), buffer.size());

	stream.close();
	LoadConfigFromMemory(buffer, ptr, layout);
}

void LoadElement(char*& addr, cfg::ConfigElement& elem, std::string& value);
void FindElement(std::string& nameBuffer, std::string& valBuffer, char*& addr, std::vector<cfg::ConfigElement> layout);

void cfg::LoadConfigFromMemory(std::vector<char> buffer, void* ptr, std::vector<ConfigElement> layout)
{
	char* ptrAddr = (char*)ptr;

	std::string nameBuffer = "";
	std::string valBuffer = "";

	size_t position = 0;
	while (position < buffer.size())
	{
		char c = buffer[position];
		if (isblank(c) || c == '\r') continue;

		if (c == '\n')
		{
			FindElement(nameBuffer, valBuffer, ptrAddr, layout);

			nameBuffer.clear();
			valBuffer.clear();
		}
		else if (c == ':')
			valBuffer.append(1, c);
		else
			nameBuffer.append(1, c);

		position++;
	}
}

void LoadElement(char*& addr, cfg::ConfigElement& elem, std::string& value)
{
	switch (elem.Type)
	{
	case cfg::ElementType::Int8: *((int8_t*)addr) = (int8_t)std::stoi(value); break;
	case cfg::ElementType::Int16: *((int16_t*)addr) = (int16_t)std::stoi(value); break;
	case cfg::ElementType::Int32: *((int32_t*)addr) = (int32_t)std::stoi(value); break;

	case cfg::ElementType::Uint8: *((uint8_t*)addr) = (uint8_t)std::stoul(value); break;
	case cfg::ElementType::Uint16: *((uint16_t*)addr) = (uint16_t)std::stoul(value); break;
	case cfg::ElementType::Uint32: *((uint32_t*)addr) = (uint32_t)std::stoul(value); break;

	case cfg::ElementType::Float: *((float*)addr) = (float)std::stof(value); break;
	case cfg::ElementType::Double: *((double*)addr) = (double)std::stod(value); break;

	case cfg::ElementType::String: (*((std::string*)addr)).assign(value); break;

	default: break;
	}

	addr += s_MemorySizes[(uint8_t)elem.Type];
}

char* GetArrayPointer(char*& addr, cfg::ConfigElement& elem);

void FindElement(std::string& nameBuffer, std::string& valBuffer, char*& addr, std::vector<cfg::ConfigElement> layout)
{
	const char* namePtr = nameBuffer.c_str();
	for (cfg::ConfigElement& elem : layout)
	{
		if (std::strcmp(elem.Name, namePtr) == 0)
		{
			if (elem.IsVector)
			{
				if (valBuffer[0] != '[')
					break;

				size_t position = 1;
				size_t start = 1;

				while (valBuffer[position] != ']')
				{
					char c = valBuffer[position];
					if (c == '"')
					{
						while (valBuffer[++position] != '"');
					}

					if (valBuffer[position] == ',')
					{
						std::string buffer = valBuffer.substr(start, position - start - 1);

						char* addrNew = GetArrayPointer(addr, elem);
						LoadElement(addrNew, elem, buffer);

						start = position;
					}
					
					position++;
				}
			}
			else
				LoadElement(addr, elem, valBuffer);

			break;
		}
	}
}

char* GetArrayPointer(char*& addr, cfg::ConfigElement& elem)
{
	switch (elem.Type)
	{
	case cfg::ElementType::Int8:
	{
		std::vector<int8_t>& vec = *((std::vector<int8_t>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}
	case cfg::ElementType::Int16:
	{
		std::vector<int16_t>& vec = *((std::vector<int16_t>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}
	case cfg::ElementType::Int32:
	{
		std::vector<int32_t>& vec = *((std::vector<int32_t>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}

	case cfg::ElementType::Uint8:
	{
		std::vector<uint8_t>& vec = *((std::vector<uint8_t>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}
	case cfg::ElementType::Uint16:
	{
		std::vector<uint16_t>& vec = *((std::vector<uint16_t>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}
	case cfg::ElementType::Uint32:
	{
		std::vector<uint32_t>& vec = *((std::vector<uint32_t>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}

	case cfg::ElementType::Float:
	{
		std::vector<float>& vec = *((std::vector<float>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}
	case cfg::ElementType::Double:
	{
		std::vector<double>& vec = *((std::vector<double>*)addr);
		vec.push_back(0);
		return (char*)&vec.back();
	}

	case cfg::ElementType::String:
	{
		std::vector<std::string>& vec = *((std::vector<std::string>*)addr);
		vec.push_back("");
		return (char*)&vec.back();
	}

	default: return nullptr;
	}
}
