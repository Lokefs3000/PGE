#pragma once

#include <queue>
#include <mutex>

namespace pge
{
	template<typename T>
	struct ThreadedQueue
	{
	public:
		std::queue<T> m_Queue;
		std::mutex m_Mutex;
	public:
		ThreadedQueue();
		~ThreadedQueue();

		T& front();
		T& back();

		void pop();

		void push(const T& item);
		void push(T&& item);

		size_t size();
		bool empty();
	};

	template<typename T>
	inline ThreadedQueue<T>::ThreadedQueue()
	{
	}

	template<typename T>
	inline ThreadedQueue<T>::~ThreadedQueue()
	{
	}

	template<typename T>
	inline T& ThreadedQueue<T>::front()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		return m_Queue.front();
	}

	template<typename T>
	inline T& ThreadedQueue<T>::back()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		return m_Queue.back();
	}

	template<typename T>
	inline void ThreadedQueue<T>::pop()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Queue.pop();
	}

	template<typename T>
	inline void ThreadedQueue<T>::push(const T& item)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Queue.push(item);
	}

	template<typename T>
	inline void ThreadedQueue<T>::push(T&& item)
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		m_Queue.push(item);
	}

	template<typename T>
	inline size_t ThreadedQueue<T>::size()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		return m_Queue.size();
	}

	template<typename T>
	inline bool ThreadedQueue<T>::empty()
	{
		std::unique_lock<std::mutex> lock(m_Mutex);
		return m_Queue.empty();
	}
}