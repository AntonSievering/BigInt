#pragma once
#include <chrono>
#include <cstdint>

namespace Engine
{
	class Timer
	{
	public:
		using clock      = std::chrono::system_clock;
		using time_point = std::chrono::time_point<clock>;

	private:
		time_point m_tpStart{};

	public:
		Timer() noexcept = default;

	public:
		Timer start() noexcept
		{
			m_tpStart = clock::now();
			return *this;
		}

		float getElapsedTime() noexcept
		{
			int64_t nNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(clock::now() - m_tpStart).count();
			return (float)nNanos / 1e9f;
		}
	};
}
