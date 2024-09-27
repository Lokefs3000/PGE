#include "pch.h"
#include "SmallString.hpp"

#include <iostream>

//#define SMALLSTRING_NULL_TERMINATOR

/*pge::SmallString::SmallString()
{
	m_String = nullptr;
	m_Length = 0;
}

pge::SmallString::SmallString(const char* string)
{
	m_Length = 0;
	while (string[m_Length] != '\0')
	{
		m_Length++;
	}

#if SMALLSTRING_NULL_TERMINATOR
	bool hasTerminator = string[m_Length - 1] != '\0';
#else
	bool hasTerminator = true;
#endif

	m_Length += hasTerminator;
	m_String = new char[m_Length];
	std::memset((char*)m_String, 0, m_Length);
	std::memcpy((void*)m_String, string, m_Length - hasTerminator);
}

pge::SmallString::SmallString(const char* string, uint32_t length)
{
#if SMALLSTRING_NULL_TERMINATOR
	bool hasTerminator = string[m_Length - 1] != '\0';
#else
	bool hasTerminator = true;
#endif

	m_Length = length + hasTerminator;
	m_String = new char[m_Length];
	std::memset((char*)m_String, 0, m_Length);
	std::memcpy((void*)m_String, string, m_Length - hasTerminator);
}

pge::SmallString::SmallString(std::string_view view)
{
#if SMALLSTRING_NULL_TERMINATOR
	bool hasTerminator = view.data()[view.size() - 1] != '\0';
#else
	bool hasTerminator = true;
#endif

	m_Length = view.size() + hasTerminator;
	m_String = new char[m_Length];
	std::memset((char*)m_String, 0, m_Length);
	std::memcpy((void*)m_String, view.data(), m_Length - hasTerminator);
}

pge::SmallString::SmallString(std::string view)
	: SmallString::SmallString(view.data(), view.size())
{
}

pge::SmallString::~SmallString()
{
	std::cout << "Deleting string: " << (m_String == nullptr ? "nullptr" : m_String) << std::endl;

	if (m_String != nullptr)
		delete[] m_String;

	m_String = nullptr;
	m_Length = 0;
}

pge::SmallString::SmallString(const SmallString& other) noexcept
{
#if SMALLSTRING_NULL_TERMINATOR
	bool hasTerminator = other.m_String[other.m_Length - 1] != '\0';
#else
	bool hasTerminator = true;
#endif

	m_Length = other.m_Length + hasTerminator;
	m_String = new char[m_Length];
	std::memset((char*)m_String, 0, m_Length);
	std::memcpy((void*)m_String, other.m_String, m_Length - hasTerminator);
}

pge::SmallString& pge::SmallString::operator=(const SmallString& other) noexcept
{
	if (m_String != nullptr)
		delete[] m_String;

	if (other.m_String == nullptr)
	{
		m_Length = 0;
		m_String = nullptr;

		return *this;
	}

#if SMALLSTRING_NULL_TERMINATOR
	bool hasTerminator = other.m_String[other.m_Length - 1] != '\0';
#else
	bool hasTerminator = true;
#endif

	m_Length = other.m_Length + hasTerminator;
	m_String = new char[m_Length];
	std::memset((char*)m_String, 0, m_Length);
	std::memcpy((void*)m_String, other.m_String, m_Length - hasTerminator);

	return *this;
}

pge::SmallString::SmallString(SmallString&& other) noexcept
{
	m_Length = other.m_Length;
	m_String = other.m_String;

	other.m_Length = 0;
	other.m_String = nullptr;
}

pge::SmallString& pge::SmallString::operator=(SmallString&& other) noexcept
{
	SmallString::~SmallString();

	m_Length = other.m_Length;
	m_String = other.m_String;

	other.m_Length = 0;
	other.m_String = 0;

	return *this;
}

uint32_t pge::SmallString::size()
{
	return m_Length;
}

const char* pge::SmallString::c_str()
{
	return m_String;
}*/
