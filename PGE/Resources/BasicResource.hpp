#pragma once

#include <atomic>

namespace pge
{
	class BasicResource
	{
	private:
		std::atomic_uint16_t m_RefCount = 0;
	public:
		void AddRef();
		void Release();

		size_t GetRefCount();
	};
}