#pragma once

#include "Particle.h"
#include "SimpleMath.h"
#include <PrimitiveBatch.h>

class Cloth
{
public:
	Cloth(UINT width, UINT height, UINT tessellation = 10);
	~Cloth();

	void Update(float dt);
	void AddForce(DirectX::SimpleMath::Vector3 force);

	UINT GetNumParticles() const { return m_Particles.size(); }
	Particle* GetParticle(int index) const { return m_Particles[index]; }

	UINT GetTessellation() const { return m_Tessellation; }

	void DrawLines(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch);

private:
	void MakeConstraints();

private:
	//width & height of cloth, NOT number of vertices in each dimension
	UINT m_Width;
	UINT m_Height;

	UINT m_Tessellation; //number of vertices in each dimension, same for both dimensions


	std::vector<Particle*> m_Particles;
	std::vector<Constraint> m_Constraints;
};