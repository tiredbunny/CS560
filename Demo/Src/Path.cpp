#include "pch.h"
#include "Path.h"
#include <map>
#include <list>
#include <iostream>

using namespace DirectX;
using namespace SimpleMath;


void Path::Init()
{

	m_StartingPoints.push_back(Vector3(-5.0f, 0.0f, -5.0f));
	m_StartingPoints.push_back(Vector3(-4.5f, 0.0f, -3.0f));
	m_StartingPoints.push_back(Vector3(-3.0f, 0.0f, -2.0f));
	m_StartingPoints.push_back(Vector3(-2.0f, 0.0f, 0.0f));
	m_StartingPoints.push_back(Vector3(0.0f, 0.0f, 0.0f));
	m_StartingPoints.push_back(Vector3(-1.0f, 0.0f, 2.0f));
	m_StartingPoints.push_back(Vector3(1.0f, 0.0f, 3.0f));
	m_StartingPoints.push_back(Vector3(2.4f, 0.0f, 2.3f));
	m_StartingPoints.push_back(Vector3(2.0f, 0.0f, 4.0f));
	m_StartingPoints.push_back(Vector3(0.0f, 0.0f, 5.0f));

	for (auto& point : m_StartingPoints)
	{
		point *= XMFLOAT3(2.0f, 2.0f, 2.0f);
	}


	//Calculate control points using starting points
	for (auto i = 1; i < m_StartingPoints.size() - 1; ++i)
	{
		auto a = m_StartingPoints[i] + 
			(m_StartingPoints[i + 1] - m_StartingPoints[i - 1]) / 8.0f;

		auto b = m_StartingPoints[i] - 
			(m_StartingPoints[i + 1] - m_StartingPoints[i - 1]) / 8.0f;

		m_ControlPoints.emplace_back(b);
		m_ControlPoints.emplace_back(m_StartingPoints[i]);
		m_ControlPoints.emplace_back(a);

	}

	//A line is drawn between each plot point to draw the curve

	for (auto i = 1; i < m_ControlPoints.size() - 3; i += 3)
	{
		//generate 100 points to draw a smooth curve using lines
		for (auto j = 0; j < 100; ++j)
		{
			//need a "u" in [0, 1] range 
			float u = j / 99.0f;

			auto point = InterpolationFunc(u, m_ControlPoints[i], m_ControlPoints[i + 1], m_ControlPoints[i + 2], m_ControlPoints[i + 3]);

			m_PlotPoints.emplace_back(point);
		}
	}

	//array of arc length tables 
	std::vector<std::map<float, float>> tables;
	
	//build arc length table using adaptive approach
	for (int i = 1; i < m_ControlPoints.size() - 3; i += 3)
	{
		std::map<float, float>  currentTable;
		currentTable[0.0f] = 0.0f;


		std::list<std::pair<float, float>> segmentList;
		segmentList.push_back(std::make_pair(0.0f, 1.0f));

		while (!segmentList.empty())
		{
			auto& const firstElement = segmentList.front();
			auto ua = firstElement.first;
			auto ub = firstElement.second;
			auto um = (ua + ub) * 0.5f;

			auto& P0 = m_ControlPoints[i];
			auto& P1 = m_ControlPoints[i + 1];
			auto& P2 = m_ControlPoints[i + 2];
			auto& P3 = m_ControlPoints[i + 3];

			auto A = ( InterpolationFunc(ua, P0, P1, P2, P3) - InterpolationFunc(um, P0, P1, P2, P3) ).Length();
			auto B = ( InterpolationFunc(um, P0, P1, P2, P3) - InterpolationFunc(ub, P0, P1, P2, P3) ).Length();
			auto C = ( InterpolationFunc(ua, P0, P1, P2, P3) - InterpolationFunc(ub, P0, P1, P2, P3) ).Length();
			auto d = A + B - C;

			if (d > ERROR_THRESHOLD || fabsf(ua - ub) > MAX_PARAM_INTERVAL)
			{
				segmentList.push_back(std::make_pair(ua, um));
				segmentList.push_back(std::make_pair(um, ub));
				segmentList.pop_front();
			}
			else 
			{ 
				currentTable[um] = currentTable[ua] + A;
				currentTable[ub] = currentTable[um] + B;
				segmentList.pop_front();
			}
		}

		tables.push_back(currentTable);
	}

	
	//combine tables into one final table

	float maxLength = 0.0f;
	for (int tableIndex = 0; tableIndex < tables.size(); ++tableIndex)
	{
		for (auto const& currentElement : tables[tableIndex])
		{
			auto u = currentElement.first + tableIndex;
			auto length = currentElement.second + maxLength;

			TableEntry currentEntry = { u, length };

			m_FinalTable.emplace_back(currentEntry);
		}

		maxLength = m_FinalTable.back().length;
	}

	m_LastTableEntry = m_FinalTable.back();

	//Normalize table
	for (auto& element : m_FinalTable)
	{
		element.u /= m_LastTableEntry.u;
		element.length /= m_LastTableEntry.length;
	}
	
}


Vector3 Path::InterpolationFunc(float u,
	Vector3 P0, Vector3 P1,
	Vector3 P2, Vector3 P3)
{
	return (-powf(u, 3) + (3 * powf(u, 2) - (3 * u) + 1)) *  P0 +
		   ((3 * powf(u, 3)) - (6 * powf(u, 2)) + (3 * u)) * P1 +
		   ((-3 * powf(u, 3)) + (3 * powf(u, 2)))          * P2 +
		   (powf(u, 3))                                    * P3;
}

#include "imgui.h"

XMMATRIX Path::Update(DX::StepTimer& const timer)
{
	//t in [0, 1]
	float normalizedTime = (timer.GetTotalSeconds() - m_TravelBeginTime) / m_TravelDuration;
	ImGui::Text("normalizedTime : %f", normalizedTime);


	float dist = GetDistanceFromTime(normalizedTime);

	float u;
	int entryIndex;
	GetUFromDistance(dist, u, entryIndex);

	// convert u from [0,1] to [0, n] where n = number of segments
	u *= m_LastTableEntry.u;

	
	auto entriesPerSegment = m_FinalTable.size() / (m_StartingPoints.size() - 3);

	// Mapping from [0:n] to the original mapping:
	// [0:1] for first seg then [0:1] for second seg
	u -= (entryIndex / entriesPerSegment);

	// E.g. if i = 12 then 12 / 256 = 0. We are in the first segment.
	// So index of p0 in the control pt. vector is 0 * 3 + 1 = 1

	// Another example, if i = 280, 280 / 256 = 1. Second seg
	// Index of p0 in the cp vector is  1 * 3 + 1 = 7
	int index = (entryIndex / entriesPerSegment) * 3 + 1;
	auto P0 = m_ControlPoints[index];
	auto P1 = m_ControlPoints[index + 1];
	auto P2 = m_ControlPoints[index + 2];
	auto P3 = m_ControlPoints[index + 3];

	auto position = InterpolationFunc(u, P0, P1, P2, P3);

	ImGui::InputFloat3("Position", reinterpret_cast<float*>(&position), 2);


	auto deltaU = m_FinalTable[1].u - m_FinalTable[0].u;
	auto W = InterpolationFunc(u + deltaU, P0, P1, P2, P3) - InterpolationFunc(u, P0, P1, P2, P3);
	W.Normalize();

	auto U = Vector3(XMVector3Cross(XMVectorSet(0, 1, 0, 0), W));
	auto V = Vector3(XMVector3Cross(W, U));


	XMMATRIX rotation = XMMatrixSet(
		U.x, U.y, U.z, 0,
		V.x, V.y, V.z, 0,
		W.x, W.y, W.z, 0,
		0, 0, 0, 1) * XMMatrixRotationY(XM_PI);

	//loop
	if (normalizedTime > 1.0f)
	{
		m_TravelBeginTime = timer.GetTotalSeconds();
	}

	return rotation * XMMatrixTranslation(position.x, position.y, position.z);
}

float Path::GetDistanceFromTime(float t)
{
	if (0.0f < t && t < m_T1)
	{
		return (m_V0 / (2 * m_T1)) * powf(t, 2);
	}
	if (m_T1 < t && t < m_T2)
	{
		return m_V0 * (t - (m_T1 / 2.0f));
	}
	if (m_T2 < t && t < 1.0f)
	{
		return ( ( (m_V0 * (t - m_T2)) / (2.0f * (1 - m_T2)) ) * (2 - t - m_T2) ) + (m_V0 * (m_T2 - (m_T1 / 2.0f)));
	}
}

void Path::GetDistanceFromU(float u, float& outDistance, int& outIndex)
{
	for (int i = 1; i < m_FinalTable.size(); ++i)
	{
		if (m_FinalTable[i].u > u)
		{
			float lerpFactor = (u - m_FinalTable[i-1].u) / (m_FinalTable[i].u - m_FinalTable[i-1].u);

			outIndex = i;
			outDistance = std::lerp(m_FinalTable[i-1].length, m_FinalTable[i].length, lerpFactor);
			return;
		}
	}
}

void Path::GetUFromDistance(float s, float& outU, int& outIndex)
{
	float uMin = 0.0f;
	float uMax = 1.0f;

	float length;
	float u;
	int segmentIndex;
	do
	{
		u = (uMin + uMax) / 2.0f;
		GetDistanceFromU(u, length, segmentIndex);

		if (s > length) 
			uMin = u;
		else 
			uMax = u;

	} while (fabsf(s - length) > 0.0001f);

	outU = u;
	outIndex = segmentIndex;
}