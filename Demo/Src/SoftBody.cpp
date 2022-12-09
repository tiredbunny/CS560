#include "pch.h"
#include "SoftBody.h"
#include "GeometricPrimitive.h"
#include "VertexTypes.h"
#include "DebugDraw.h"

using namespace DirectX;
using namespace SimpleMath;

SoftBody::~SoftBody()
{
	for (int i = 0; i < m_Particles.size(); ++i)
	{
		delete m_Particles[i];
	}

	m_Particles.clear();
	m_Constraints.clear();
}

SoftBody::SoftBody(UINT width, UINT height, UINT tessellation) :
	m_Width(width), m_Height(height), m_Tessellation(tessellation)
{
	std::vector<GeometricPrimitive::VertexType> vertices;
	std::vector<uint16_t> indices;

	Helpers::CreateGridXY(vertices, indices, width, height, tessellation);

	for (auto& vertex : vertices)
	{
		XMStoreFloat3(
			&vertex.position,
			XMVector3Transform(XMLoadFloat3(&vertex.position), XMMatrixTranslation(0.0f, height/2, 0.0f))
		);


		Particle* p = new Particle(vertex.position);

		m_Particles.push_back(p);
	}

	MakeConstraints();

	//first two extreme points fixed on the first row
	m_Particles[0]->m_Fixed = true;
	m_Particles[tessellation - 1]->m_Fixed = true;

	//for (int i = 0; i < m_Tessellation; ++i)
	//	m_Particles[i]->m_Fixed = true;
}

void SoftBody::MakeConstraints()
{
	for (int i = 0; i < m_Tessellation; ++i)
	{
		for (int j = 0; j < m_Tessellation; ++j)
		{
			int currentRowIndex = i * m_Tessellation;

			if (j != m_Tessellation - 1)
			{
				// line to the right
				m_Constraints.push_back(Spring(m_Particles[currentRowIndex + j], m_Particles[currentRowIndex + j + 1]));
			}

			int belowRowIndex = (i + 1) * m_Tessellation;

			if (i != m_Tessellation - 1)
			{
				//line to below
				m_Constraints.push_back(Spring(m_Particles[currentRowIndex + j], m_Particles[belowRowIndex + j]));

				if (j != m_Tessellation - 1)
				{
					//diagonal 1 line
					m_Constraints.push_back(Spring(m_Particles[currentRowIndex + j], m_Particles[belowRowIndex + j + 1]));
				}

			}

			int upperRowIndex = (i - 1) * m_Tessellation;

			if (i != 0)
			{
				if (j != m_Tessellation - 1)
				{
					//diagonal 2 line
					m_Constraints.push_back(Spring(m_Particles[currentRowIndex + j], m_Particles[upperRowIndex + j + 1]));
				}

			}
		}
	}
}

void SoftBody::Update(float dt)
{
	for (int i = 0; i < 20; ++i)
	{
		for (auto& const c : m_Constraints)
		{
			c.Update();
		}
	}

	for (auto& const p : m_Particles)
	{
		p->Update(dt);
	}

}

void SoftBody::ApplyForce(DirectX::SimpleMath::Vector3 force)
{
	for (auto& const p : m_Particles)
	{
		p->AddForce(force);
	}
}

void SoftBody::DrawLines(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch)
{
	Vector3 origin;
	Vector3 direction;

	for (int i = 0; i < m_Tessellation; ++i)
	{
		for (int j = 0; j < m_Tessellation; ++j)
		{
			int currentRowIndex = i * m_Tessellation;

			if (j != m_Tessellation - 1)
			{
				// line to the right
				origin = m_Particles[currentRowIndex + j]->m_Position;
				direction = m_Particles[currentRowIndex + j + 1]->m_Position - origin;

				DX::DrawRay(batch, origin, direction, false, Colors::Red);
			}

			int belowRowIndex = (i + 1) * m_Tessellation;

			if (i != m_Tessellation - 1)
			{
				//line to below
				origin = m_Particles[currentRowIndex + j]->m_Position;
				direction = m_Particles[belowRowIndex + j]->m_Position - origin;

				DX::DrawRay(batch, origin, direction, false, Colors::Green);

				if (j != m_Tessellation - 1)
				{
					//diagonal 1 line
					origin = m_Particles[currentRowIndex + j]->m_Position;
					direction = m_Particles[belowRowIndex + j + 1]->m_Position - origin;

					DX::DrawRay(batch, origin, direction, false, Colors::Blue);
				}

			}

			int upperRowIndex = (i - 1) * m_Tessellation;

			if (i != 0)
			{
				if (j != m_Tessellation - 1)
				{
					//diagonal 2 line
					origin = m_Particles[currentRowIndex + j]->m_Position;
					direction = m_Particles[upperRowIndex + j + 1]->m_Position - origin;

					DX::DrawRay(batch, origin, direction, false, Colors::Yellow);
				}
			
			}
		}
	}
}

void SoftBody::Collision(DirectX::SimpleMath::Vector3 sphereCenter, float radius)
{
	for (auto& const p : m_Particles)
	{
		auto diff = p->m_Position - sphereCenter;
		float length = diff.Length();

		if (length < radius) 
		{
			diff.Normalize();
			p->Offset((radius - length) * diff);
		}
	}
}

Vector3 SoftBody::GetFaceNormal(Particle* p1, Particle* p2, Particle* p3)
{
	Vector3 v1 = p2->m_Position - p1->m_Position;
	Vector3 v2 = p3->m_Position - p1->m_Position;

	return v1.Cross(v2);
}

void SoftBody::ApplyWind(DirectX::SimpleMath::Vector3 force)
{
	for (int i = 0; i < m_Tessellation - 1; ++i)
	{
		for (int j = 0; j < m_Tessellation - 1; ++j)
		{
			ApplyFaceForce(GetParticle(i, j),
				GetParticle(i, j + 1),
				GetParticle(i + 1, j),
				force);

			ApplyFaceForce(GetParticle(i + 1, j + 1), 
				GetParticle(i + 1, j), 
				GetParticle(i, j + 1), 
				force);
		}
	}
}

void SoftBody::ApplyFaceForce(Particle* p1, Particle* p2, Particle* p3, Vector3 direction)
{
	Vector3 normal = GetFaceNormal(p1, p2, p3);
	normal.Normalize();

	Vector3 force = normal * normal.Dot(direction);
	p1->AddForce(force);
	p2->AddForce(force);
	p3->AddForce(force);
}