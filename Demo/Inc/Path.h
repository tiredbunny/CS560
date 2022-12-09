#pragma once

#include "SimpleMath.h"
#include "StepTimer.h"

constexpr auto ERROR_THRESHOLD = 0.01f;
constexpr auto MAX_PARAM_INTERVAL = 0.01f;

struct TableEntry
{
	float u;
	float length;
};


class Path
{
public:
	Path(float currentTime) : m_T1(0.2f), m_T2(0.8f), m_TravelDuration(10.0f),
		m_TravelBeginTime(currentTime)
	{
		m_V0 = 2.0f / (1.0f - m_T1 + m_T2);

		ComputeTable();
	}

	DirectX::SimpleMath::Vector3 InterpolationFunc(float u, 
		DirectX::SimpleMath::Vector3 P0, DirectX::SimpleMath::Vector3 P1, 
		DirectX::SimpleMath::Vector3 P2, DirectX::SimpleMath::Vector3 P3);

	DirectX::XMMATRIX Update(DX::StepTimer& const timer);
private:
	float GetDistanceFromTime(float time);
	
	//outIndex is the index in the final table
	void GetDistanceFromU(float u, float& outDistance, int& outIndex);
	void GetUFromDistance(float s, float& outU, int& outIndex);
	float GetVelocity(float time);

	void ComputeTable();
private:
	std::vector<TableEntry> m_FinalTable;
	TableEntry m_LastTableEntry;

	float m_V0;
	float m_T1, m_T2;
	float m_TravelDuration; //total time it takes to travel on the path
	float m_TravelBeginTime;

public:
	std::vector<DirectX::SimpleMath::Vector3> m_PlotPoints;
	std::vector<DirectX::SimpleMath::Vector3> m_StartingPoints;
	std::vector<DirectX::SimpleMath::Vector3> m_ControlPoints;

	//can also be used to indicate how much path is covered
	float m_NormalizedTime; // [0, 1]

	float m_SpeedFactor; //[0, 1] & 1 = full velocity
};

