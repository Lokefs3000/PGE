#pragma once

#include <chrono>

namespace pge
{
	struct Stopwatch
	{
	private:
		std::chrono::high_resolution_clock::time_point m_Start;
		double m_Duration;
		bool m_IsRunning;
	public:
		Stopwatch(bool autoRun = false);

		void Start();
		void Stop();
		void Restart();

		double GetDuration();
		bool IsRunning();
	};
}