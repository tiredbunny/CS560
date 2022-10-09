#include "pch.h"
#include "Timer.h"

Timer::Timer() :
	m_CurrentTime(0), m_PreviousTime(0),
	m_DeltaTime(0)
{
	__int64 countsPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSecond);
	
	m_SecondsPerCount = 1.0 / (double)countsPerSecond;

	Reset();
}

void Timer::Reset()
{
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	m_PreviousTime = currentTime;
}

void Timer::Tick()
{
	__int64 currentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	m_CurrentTime = currentTime;
	m_DeltaTime = (m_CurrentTime - m_PreviousTime) * m_SecondsPerCount;
	m_PreviousTime = m_CurrentTime;

	if (m_DeltaTime < 0.0)
		m_DeltaTime = 0.0;
}