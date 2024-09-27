#include "pch.h"
#include "Stopwatch.hpp"

pge::Stopwatch::Stopwatch(bool autoRun)
{
	if (autoRun) Start();
}

void pge::Stopwatch::Start()
{
	if (!m_IsRunning)
	{
		m_IsRunning = true;
		m_Start = std::chrono::high_resolution_clock::now();
	}
}

void pge::Stopwatch::Stop()
{
	if (m_IsRunning)
	{
		m_IsRunning = false;
		m_Start = std::chrono::high_resolution_clock::now();

		m_Duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::high_resolution_clock::now() - m_Start).count();
	}
}

void pge::Stopwatch::Restart()
{
	m_Start = std::chrono::high_resolution_clock::now();
}

double pge::Stopwatch::GetDuration()
{
	if (m_IsRunning)
		return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::high_resolution_clock::now() - m_Start).count();
	else
		return m_Duration;
}

bool pge::Stopwatch::IsRunning()
{
	return m_IsRunning;
}
