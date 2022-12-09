#pragma once

#include <SimpleMath.h>

class Particle
{
public:
	Particle(DirectX::SimpleMath::Vector3 position) :
		m_Position(position), m_Mass(1.0f), 
	    m_Velocity(), m_Force() {}

	void Update(float dt);
	void AddForce(DirectX::SimpleMath::Vector3 force);

	void Offset(DirectX::SimpleMath::Vector3 offset);

public:
	DirectX::SimpleMath::Vector3 m_Force;
	DirectX::SimpleMath::Vector3 m_Position;
	DirectX::SimpleMath::Vector3 m_Velocity;
	float m_Mass;

	bool m_Fixed = false;
};

class Constraint
{
public:
	Constraint(Particle* p1, Particle* p2) :
		m_P1(p1), m_P2(p2)
	{
		m_RestDistance = (p1->m_Position - p2->m_Position).Length();
	}

	void Update()
	{
		auto p1ToP2 = m_P2->m_Position - m_P1->m_Position;

		float currentDistance = p1ToP2.Length();

		auto correctionVector = p1ToP2 * (1.0f - m_RestDistance / currentDistance);
		correctionVector *= 0.5f; 

		m_P1->Offset(correctionVector);
		m_P2->Offset(-correctionVector);
	}

private:
	Particle* m_P1;
	Particle* m_P2;
	float m_RestDistance;
};