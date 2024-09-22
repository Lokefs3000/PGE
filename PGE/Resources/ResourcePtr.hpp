#pragma once

#include "BasicResource.hpp"

namespace pge
{
	template<typename TUnkown>
	struct ResourcePtr
	{
	private:
		BasicResource* m_Ptr;
	public:
		ResourcePtr() noexcept;
		ResourcePtr(TUnkown* ptr) noexcept;
		ResourcePtr(const ResourcePtr& other) noexcept;
		~ResourcePtr() noexcept;

		void Reset() noexcept;

		TUnkown* Get();
		size_t RefCount();

		TUnkown* operator->();
	};

	template<typename TUnkown>
	inline ResourcePtr<TUnkown>::ResourcePtr() noexcept
	{
		m_Ptr = nullptr;
	}

	template<typename TUnkown>
	inline ResourcePtr<TUnkown>::ResourcePtr(TUnkown* ptr) noexcept
	{
		m_Ptr = (BasicResource*)ptr;
		if (m_Ptr != nullptr)
			m_Ptr->AddRef();
	}

	template<typename TUnkown>
	inline ResourcePtr<TUnkown>::ResourcePtr(const ResourcePtr& other) noexcept
	{
		m_Ptr = other.m_Ptr;
		if (m_Ptr != nullptr)
			m_Ptr->AddRef();
	}

	template<typename TUnkown>
	inline ResourcePtr<TUnkown>::~ResourcePtr() noexcept
	{
		Reset();
	}

	template<typename TUnkown>
	inline void ResourcePtr<TUnkown>::Reset() noexcept
	{
		if (m_Ptr != nullptr)
			m_Ptr->Release();
		m_Ptr = nullptr;
	}

	template<typename TUnkown>
	inline TUnkown* ResourcePtr<TUnkown>::Get()
	{
		return (TUnkown*)m_Ptr;
	}

	template<typename TUnkown>
	inline size_t ResourcePtr<TUnkown>::RefCount()
	{
		return m_Ptr->GetRefCount();
	}

	template<typename TUnkown>
	inline TUnkown* ResourcePtr<TUnkown>::operator->()
	{
		return (TUnkown*)m_Ptr;
	}
}