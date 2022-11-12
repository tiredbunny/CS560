#pragma once

#include "SimpleMath.h"


class Path
{
public:
	void Init();
	
	DirectX::SimpleMath::Vector3 InterpolationFunc(float u, std::vector<DirectX::SimpleMath::Vector3> p);


public:
	std::vector<DirectX::SimpleMath::Vector3> m_PlotPoints;
	std::vector<DirectX::SimpleMath::Vector3> m_StartingPoints;

};