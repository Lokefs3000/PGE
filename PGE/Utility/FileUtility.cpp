#include "pch.h"
#include "FileUtility.hpp"

bool pge::FileUtility::ReadFile(std::string_view path, std::vector<char>& outBuffer)
{
    std::ifstream fileStream(path.data(), std::ios::ate | std::ios::binary);
    if (!fileStream.is_open() || fileStream.bad())
        return false;

    outBuffer.resize(fileStream.tellg());
    fileStream.seekg(0);
    fileStream.read(outBuffer.data(), outBuffer.size());
    fileStream.close();

    return true;
}

bool pge::FileUtility::WriteFile(std::string_view path, std::vector<char>& inBuffer)
{
    std::ofstream fileStream(path.data(), std::ios::binary | std::ios::trunc);
    if (!fileStream.is_open() || fileStream.bad())
        return false;

    fileStream.write(inBuffer.data(), inBuffer.size());
    fileStream.close();

    return true;
}

bool pge::FileUtility::ReadFile(std::filesystem::path path, std::vector<char>& outBuffer)
{
    std::ifstream fileStream(path, std::ios::ate | std::ios::binary);
    if (!fileStream.is_open() || fileStream.bad())
        return false;

    outBuffer.resize(fileStream.tellg());
    fileStream.seekg(0);
    fileStream.read(outBuffer.data(), outBuffer.size());
    fileStream.close();

    return true;
}

bool pge::FileUtility::WriteFile(std::filesystem::path path, std::vector<char>& inBuffer)
{
    std::ofstream fileStream(path, std::ios::binary | std::ios::trunc);
    if (!fileStream.is_open() || fileStream.bad())
        return false;

    fileStream.write(inBuffer.data(), inBuffer.size());
    fileStream.close();

    return true;
}

std::fstream pge::FileUtility::OpenStream(std::string_view path, StreamDirection direction, bool clear)
{
    std::fstream fileStream(path.data(), ((direction == StreamDirection::Out) ? std::ios::out : std::ios::in) | std::ios::binary | (clear ? std::ios::trunc : 0));
    return std::move(fileStream);
}

std::fstream pge::FileUtility::OpenStream(std::filesystem::path path, StreamDirection direction, bool clear)
{
    std::fstream fileStream(path, ((direction == StreamDirection::Out) ? std::ios::out : std::ios::in) | std::ios::binary | (clear ? std::ios::trunc : 0));
    return std::move(fileStream);
}
