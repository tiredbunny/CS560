#include "pch.h"
#include "Path.h"

using namespace DirectX;
using namespace SimpleMath;

void Path::Init()
{
	m_StartingPoints.emplace_back(Vector3(-5.0f, 0.0f, -5.0f));
	m_StartingPoints.emplace_back(Vector3(-5.0f, 0.0f, -3.0f));
	m_StartingPoints.emplace_back(Vector3(-3.0f, 0.0f, -2.0f));
	m_StartingPoints.emplace_back(Vector3(-2.0f, 0.0f, 0.0f));
	m_StartingPoints.emplace_back(Vector3(0.0f, 0.0f, 0.0f));
	m_StartingPoints.emplace_back(Vector3(3.0f, 0.0f, 3.0f));
	m_StartingPoints.emplace_back(Vector3(2.0f, 0.0f, -4.0f));
	m_StartingPoints.emplace_back(Vector3(0.0f, 0.0f, -5.0f));

	for (auto& point : m_StartingPoints)
	{
		point *= XMFLOAT3(2.0f, 2.0f, 2.0f);
	}

	std::vector<Vector3> controlPoints;

	//Calculate control points using starting points
	for (auto i = 1; i < m_StartingPoints.size() - 1; ++i)
	{
		auto a = m_StartingPoints[i] + 
			(m_StartingPoints[i + 1] - m_StartingPoints[i - 1]) / 8.0f;

		auto b = m_StartingPoints[i] - 
			(m_StartingPoints[i + 1] - m_StartingPoints[i - 1]) / 8.0f;

		controlPoints.emplace_back(b);
		controlPoints.emplace_back(m_StartingPoints[i]);
		controlPoints.emplace_back(a);

	}

	//A line is drawn between each plot point to draw the curve
	std::vector<Vector3> temp(4);
	for (auto i = 1; i < controlPoints.size() - 3; i += 3)
	{
		temp[0] = controlPoints[i];
		temp[1] = controlPoints[i + 1];
		temp[2] = controlPoints[i + 2];
		temp[3] = controlPoints[i + 3];

		//100 points between 
		for (auto j = 0; j < 100; ++j)
		{
			//convert u to [0, 1] range for the interpolation function
			float u = j / 99.0f;

			auto point = InterpolationFunc(u, temp);

			m_PlotPoints.emplace_back(point);
		}
	}
}

Vector3 Path::InterpolationFunc(float u,  std::vector<DirectX::SimpleMath::Vector3> P)
{
	return (-powf(u, 3) + (3 * powf(u, 2) - (3 * u) + 1)) *  P[0] +
		   ((3 * powf(u, 3)) - (6 * powf(u, 2)) + (3 * u)) * P[1] +
		   ((-3 * powf(u, 3)) + (3 * powf(u, 2)))          * P[2] +
		   (powf(u, 3))                                         * P[3];
}
