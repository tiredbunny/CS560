#include "pch.h"
#include "Particle.h"

using namespace DirectX;
using namespace SimpleMath;

void Particle::Update(float dt)
{
	if (m_Fixed)
		return;
    
    //===============  RK4 integration =======================//

    auto acc = m_Force / m_Mass;

    auto k1 = Derivative(m_Velocity, acc);
    auto k2 = Derivative(m_Velocity + k1.m_DV * dt * 0.5f, acc);
    auto k3 = Derivative(m_Velocity + k2.m_DV * dt * 0.5f, acc);
    auto k4 = Derivative(m_Velocity + k3.m_DV * dt, acc);

    auto dxdt = 1.f / 6.f * (k1.m_DP + 2.f * (k2.m_DP + k3.m_DP) + k4.m_DP);
    auto dvdt = 1.f / 6.f * (k1.m_DV + 2.f * (k2.m_DV + k3.m_DV) + k4.m_DV);

    m_Position += dxdt * dt;
    m_Velocity += dvdt * dt;

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
