#include "pch.h"

using namespace DirectX;

void Helpers::CreateGrid(std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices, 
	const uint16_t width, const uint16_t depth, uint16_t tessellation)
{
	vertices.clear();
	indices.clear();

	UINT vertexCount = tessellation * tessellation;
	UINT quadCount = (tessellation - 1) * (tessellation - 1);
	UINT faceCount = quadCount * 2;

	float quadSizeX = static_cast<float>(width / (tessellation - 1));
	float quadSizeZ = static_cast<float>(depth / (tessellation - 1));

	float quadU = 1.0f / (tessellation - 1);
	float quadV = 1.0f / (tessellation - 1);

	float halfWidth = 0.5f * static_cast<float>(width);
	float halfDepth = 0.5f * static_cast<float>(depth);

	vertices.resize(vertexCount);

	for (int i = 0; i < tessellation; ++i)
	{
		float z = halfDepth - i * quadSizeZ;

		for (int j = 0; j < tessellation; ++j)
		{
			float x = -halfWidth + j * quadSizeX;

			int rowIndex = i * tessellation;

			vertices[rowIndex + j].position = XMFLOAT3(x, 0.0f, z);
			vertices[rowIndex + j].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			vertices[rowIndex + j].textureCoordinate = XMFLOAT2(quadU * j, quadV * i);
		}
	}

	indices.resize(quadCount * 6);

	UINT k = 0;
	for (int i = 0; i < tessellation - 1; ++i)
	{
		for (int j = 0; j < tessellation - 1; ++j)
		{
			int rowIndex = i * tessellation;
			int nextRowIndex = (i + 1) * tessellation;

			indices[k] = rowIndex + j;
			indices[k + 1] = rowIndex + j + 1;
			indices[k + 2] = nextRowIndex + j;
			indices[k + 3] = nextRowIndex + j;
			indices[k + 4] = rowIndex + j + 1;
			indices[k + 5] = nextRowIndex + j + 1;

			k += 6;
		}
	}
}
