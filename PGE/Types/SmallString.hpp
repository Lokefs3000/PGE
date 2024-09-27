#pragma once

#include <string>

namespace pge
{
	//not thread-safe
	/*class SmallString
	{
	private:
		const char* m_String;
		uint32_t m_Length;
	public:
		SmallString();
		SmallString(const char* string); //semi-unsafe
		SmallString(const char* string, uint32_t length);

		SmallString(std::string_view view);
		SmallString(std::string view);

		~SmallString();

		SmallString(const SmallString& other) noexcept; //copy
		SmallString& operator=(const SmallString& other) noexcept; //copy

		SmallString(SmallString&& other) noexcept; //move
		SmallString& operator=(SmallString&& other) noexcept;

		uint32_t size();
		const char* c_str();

		bool operator==(const char* other) noexcept { return strncmp(m_String, other, m_Length - 1) == 0; }
		bool operator==(const SmallString& other) noexcept { return strncmp(m_String, other.m_String, m_Length - 1) == 0; }

		bool operator<(const SmallString& other) noexcept { return m_String < other.m_String; }

		operator std::string_view() const noexcept { return std::string_view(m_String, m_Length - 1); }
		operator std::string() const noexcept { return std::string(m_String, m_Length); }
	};*/

	typedef std::string SmallString;
}