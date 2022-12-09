#pragma once

#include <SimpleMath.h>

class Particle
{
	struct Derivative
	{
		DirectX::SimpleMath::Vector3 m_DP;
		DirectX::SimpleMath::Vector3 m_DV;

		Derivative(DirectX::SimpleMath::Vector3 dp, DirectX::SimpleMath::Vector3 dv) :
			m_DP(dp), m_DV(dv) {}
		Derivative() : Derivative({}, {})
		{}

	};

public:
	Particle(DirectX::SimpleMath::Vector3 position) :
		m_Position(position), m_Mass(1.0f), 
	    m_Velocity(), m_Force() {}
	
	Particle() :
		Particle({ 0,0,0 })
	{}

	void Update(float dt);
	void AddForce(DirectX::SimpleMath::Vector3 force);

	void Offset(DirectX::SimpleMath::Vector3 offset);
	
public:
	DirectX::SimpleMath::Vector3 m_Force;
	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Vector3 m_Velocity;

	float m_Mass = 1.0f;
	bool m_Fixed = false;
};




class Spring
{
public:
	Spring(Particle* p1, Particle* p2) :
		m_P1(p1), m_P2(p2)
	{
		m_RestLength = (p1->m_Position - p2->m_Position).Length();
	}

	void Update()
	{
		auto diff = m_P1->m_Position - m_P2->m_Position;
		auto length = diff.Length();

		auto vec = (diff * (1.0f - m_RestLength / length)) * 0.3f;
		m_P2->Offset(vec);
		m_P1->Offset(-vec);
	}

	
private:
	Particle* m_P1;
	Particle* m_P2;
	float m_RestLength;
};