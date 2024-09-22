#pragma once

#include <vector>
#include <functional>

template<typename... Args>
struct Signal
{
public:
	using FnProc = std::function<void(Args...)>;

private:
	std::vector<FnProc> m_Connected;

	size_t DoesExist(FnProc proc);
public:
	void operator+=(FnProc proc);
	void operator-=(FnProc proc);

	void operator()(Args... args);
};

template<typename ...Args>
inline size_t Signal<Args...>::DoesExist(FnProc proc)
{
	for (size_t i = 0; i < m_Connected.size(); i++)
	{
		if (m_Connected[i].target_type() == proc.target_type())
			return i;
	}

	return -1;
}

template<typename ...Args>
inline void Signal<Args...>::operator+=(FnProc proc)
{
	if (DoesExist(proc) == -1)
		m_Connected.push_back(proc);
}

template<typename ...Args>
inline void Signal<Args...>::operator-=(FnProc proc)
{
	if (size_t idx = DoesExist(proc) != -1)
		m_Connected.erase(m_Connected.begin() + idx);
}

template<typename ...Args>
inline void Signal<Args...>::operator()(Args ...args)
{
	for (FnProc& proc : m_Connected)
		proc(std::forward<Args...>(args...));
}
