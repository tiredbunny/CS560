#include "pch.h"
#include "Cloth.h"
#include "GeometricPrimitive.h"
#include "VertexTypes.h"
#include "DebugDraw.h"

using namespace DirectX;
using namespace SimpleMath;

Cloth::~Cloth()
{
	for (int i = 0; i < m_Particles.size(); ++i)
	{
		delete m_Particles[i];
	}

	m_Particles.clear();
	m_Constraints.clear();
}

Cloth::Cloth(UINT width, UINT height, UINT tessellation) :
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

	m_Particles[0]->m_Fixed = true;
	m_Particles[tessellation - 1]->m_Fixed = true;
}

void Cloth::MakeConstraints()
{
	for (int i = 0; i < m_Tessellation; ++i)
	{
		for (int j = 0; j < m_Tessellation; ++j)
		{
			int currentRowIndex = i * m_Tessellation;

			if (j != m_Tessellation - 1)
			{
				// line to the right
				m_Constraints.push_back(Constraint(m_Particles[currentRowIndex + j], m_Particles[currentRowIndex + j + 1]));
			}

			int belowRowIndex = (i + 1) * m_Tessellation;

			if (i != m_Tessellation - 1)
			{
				//line to below
				m_Constraints.push_back(Constraint(m_Particles[currentRowIndex + j], m_Particles[belowRowIndex + j]));

				if (j != m_Tessellation - 1)
				{
					//diagonal 1 line
					m_Constraints.push_back(Constraint(m_Particles[currentRowIndex + j], m_Particles[belowRowIndex + j + 1]));
				}

			}

			int upperRowIndex = (i - 1) * m_Tessellation;

			if (i != 0)
			{
				if (j != m_Tessellation - 1)
				{
					//diagonal 2 line
					m_Constraints.push_back(Constraint(m_Particles[currentRowIndex + j], m_Particles[upperRowIndex + j + 1]));
				}

			}
		}
	}
}

void Cloth::Update(float dt)
{
	for (auto& const p : m_Particles)
	{
		p->Update(dt);
	}

	for (int i = 0; i < 15; ++i)
	{
		for (auto& const c : m_Constraints)
		{
			c.Update();
		}
	}
	
}

void Cloth::AddForce(DirectX::SimpleMath::Vector3 force)
{
	for (auto& const p : m_Particles)
	{
		p->AddForce(force);
	}
}

void Cloth::DrawLines(DirectX::PrimitiveBatch<DirectX::VertexPositionColor>* batch)
{
	for (int i = 0; i < m_Tessellation; ++i)
	{
		for (int j = 0; j < m_Tessellation; ++j)
		{
			int currentRowIndex = i * m_Tessellation;

			if (j != m_Tessellation - 1)
			{
				// line to the right
				Vector3 origin = m_Particles[currentRowIndex + j]->m_Position;
				Vector3 direction = m_Particles[currentRowIndex + j + 1]->m_Position - origin;

				DX::DrawRay(batch, origin, direction, false, Colors::Red);
			}

			int belowRowIndex = (i + 1) * m_Tessellation;

			if (i != m_Tessellation - 1)
			{
				//line to below
				Vector3 origin = m_Particles[currentRowIndex + j]->m_Position;
				Vector3 direction = m_Particles[belowRowIndex + j]->m_Position - origin;

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
					Vector3 origin = m_Particles[currentRowIndex + j]->m_Position;
					Vector3 direction = m_Particles[upperRowIndex + j + 1]->m_Position - origin;

					DX::DrawRay(batch, origin, direction, false, Colors::Yellow);
				}
			
			}
		}
	}
}

