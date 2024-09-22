#include "pch.h"
#include "BasicResource.hpp"

#ifdef _DEBUG
#include <spdlog/spdlog.h>
#endif

void pge::BasicResource::AddRef()
{
	m_RefCount++;
}

void pge::BasicResource::Release()
{
	m_RefCount--;
	if (m_RefCount <= 0)
	{
#ifdef _DEBUG
		spdlog::get("PGE")->debug("Deleting unreferneced resource: {}", typeid(this).name());
#endif

		delete this;
		m_RefCount = UINT64_MAX;
	}
}

size_t pge::BasicResource::GetRefCount()
{
	return m_RefCount;
}
