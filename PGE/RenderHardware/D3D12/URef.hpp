#pragma once

#include <Unknwnbase.h>

namespace pge
{
	template<typename TUnkown>
	struct URef
	{
	private:
		IUnknown* m_Ptr;
	public:
		URef() noexcept;
		URef(TUnkown* ptr) noexcept;
		URef(const URef& other) noexcept;
		~URef() noexcept;

		void Reset() noexcept;

		TUnkown* Get();
		TUnkown** GetAddressOf();
		void** GetVoidAddressOf();
		GUID GetGuidOf();

		TUnkown* operator->();
	};

	template<typename TUnkown>
	inline URef<TUnkown>::URef() noexcept
	{
		m_Ptr = nullptr;
	}

	template<typename TUnkown>
	inline URef<TUnkown>::URef(TUnkown* ptr) noexcept
	{
		m_Ptr = (IUnknown*)ptr;
		if (m_Ptr != nullptr)
			m_Ptr->AddRef();
	}

	template<typename TUnkown>
	inline URef<TUnkown>::URef(const URef& other) noexcept
	{
		m_Ptr = other.m_Ptr;
		if (m_Ptr != nullptr)
			m_Ptr->AddRef();
	}

	template<typename TUnkown>
	inline URef<TUnkown>::~URef() noexcept
	{
		Reset();
	}

	template<typename TUnkown>
	inline void URef<TUnkown>::Reset() noexcept
	{
		if (m_Ptr != nullptr)
			m_Ptr->Release();
		m_Ptr = nullptr;
	}

	template<typename TUnkown>
	inline TUnkown* URef<TUnkown>::Get()
	{
		return (TUnkown*)m_Ptr;
	}

	template<typename TUnkown>
	inline TUnkown** URef<TUnkown>::GetAddressOf()
	{
		return (TUnkown**)&m_Ptr;
	}

	template<typename TUnkown>
	inline void** URef<TUnkown>::GetVoidAddressOf()
	{
		return (void**)&m_Ptr;
	}

	template<typename TUnkown>
	inline GUID URef<TUnkown>::GetGuidOf()
	{
		return __uuidof(TUnkown);
	}

	template<typename TUnkown>
	inline TUnkown* URef<TUnkown>::operator->()
	{
		return (TUnkown*)m_Ptr;
	}
}