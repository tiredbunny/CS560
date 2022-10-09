#pragma once


class Timer
{
private:
	__int64 m_CurrentTime;
	__int64 m_PreviousTime;

	double m_DeltaTime;
	double m_SecondsPerCount;
public:
	Timer();

	void Tick();
	void Reset();

	float GetDeltaTime() const { return static_cast<float>(m_DeltaTime); }
};