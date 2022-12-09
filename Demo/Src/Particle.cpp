#include "pch.h"
#include "Particle.h"

using namespace DirectX;
using namespace SimpleMath;

void Particle::Update(float dt)
{
	if (m_Fixed)
		return;

	auto acceleration = m_Force / m_Mass;

	m_Velocity += acceleration * dt;
	m_Position += m_Velocity * dt;

	m_Force = Vector3(0, 0, 0);
}

void Particle::AddForce(DirectX::SimpleMath::Vector3 force)
{
	m_Force += force;
}

void Particle::Offset(DirectX::SimpleMath::Vector3 offset)
{
	if (m_Fixed)
		return;

	m_Position += offset;
}
