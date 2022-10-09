#include "pch.h"
#include "Vertex.h"

const D3D11_INPUT_ELEMENT_DESC SimpleVertex::InputElements[SimpleVertex::ElementCount] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static_assert(sizeof(SimpleVertex) == 12, "mismatch");

const D3D11_INPUT_ELEMENT_DESC TreePointSprite::InputElements[TreePointSprite::ElementCount] =
{
	{ "CENTER", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "SIZE",   0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

static_assert(sizeof(TreePointSprite) == 20, "mismatch");


//struct VertexIn
//{
//	float3 PosL : SV_POSITION;
//	float3 NormalL : NORMAL0;
//	float2 Tex : TEXCOORD0;
//	float3 Weights : WEIGHTS;
//	uint4 BoneIndices : BONEINDICES;
//};

const D3D11_INPUT_ELEMENT_DESC SkinnedVertex::InputElements[SkinnedVertex::ElementCount] =
{
	{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD",   0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{"BONEINDICES",  0, DXGI_FORMAT_R8G8B8A8_UINT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

static_assert(sizeof(SkinnedVertex) == 48, "mismatch");