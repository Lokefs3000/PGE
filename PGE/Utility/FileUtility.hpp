#pragma once

#include <string_view>
#include <vector>
#include <filesystem>
#include <fstream>

namespace pge
{
	enum class StreamDirection
	{
		In,
		Out
	};

	class FileUtility
	{
	public:
		static bool ReadFile(std::string_view path, std::vector<char>& outBuffer);
		static bool WriteFile(std::string_view path, std::vector<char>& inBuffer);

		static bool ReadFile(std::filesystem::path path, std::vector<char>& outBuffer);
		static bool WriteFile(std::filesystem::path path, std::vector<char>& inBuffer);

		static std::fstream OpenStream(std::string_view path, StreamDirection direction, bool clear = false);
		static std::fstream OpenStream(std::filesystem::path path, StreamDirection direction, bool clear = false);
	};
}