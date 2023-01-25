#include "pch.h"
#include "Vertex.h"



const D3D11_INPUT_ELEMENT_DESC SkyVertex::InputElements[SkyVertex::ElementCount] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
static_assert(sizeof(SkyVertex) == 12, "mismatch");
