#pragma once

#include "Particle.h"
#include "SimpleMath.h"
#include <PrimitiveBatch.h>
#include "imgui.h"


class SoftBody
{
public:
	SoftBody(UINT width, UINT height, UINT tessellation = 10);
	~SoftBody();

	void Update(float dt);
	void ApplyForce(DirectX::SimpleMath::Vector3 force);
	void ApplyWind(DirectX::SimpleMath::Vector3 force);

	void DrawLines(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch);
	void Collision(DirectX::SimpleMath::Vector3 sphereCenter, float radius);

	UINT GetNumParticles() const { return m_Particles.size(); }
	Particle* GetParticle(int index) const { return m_Particles[index]; }
	Particle* GetParticle(int x, int y) const { return m_Particles[y * m_Tessellation + x]; }
	UINT GetTessellation() const { return m_Tessellation; }

private:
	void MakeConstraints();
	DirectX::SimpleMath::Vector3 GetFaceNormal(Particle* v1, Particle* v2, Particle* v3);
	void ApplyFaceForce(Particle* v1, Particle* v2, Particle* v3, DirectX::SimpleMath::Vector3 direction);

private:
	//width & height of cloth, NOT number of vertices in each dimension
	UINT m_Width;
	UINT m_Height;

	UINT m_Tessellation; //number of vertices in each dimension, same for both dimensions

	std::vector<Particle*> m_Particles;
	std::vector<Spring> m_Constraints;
};

