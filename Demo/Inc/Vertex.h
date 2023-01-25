#pragma once

struct SkyVertex
{
	DirectX::XMFLOAT3 Position;

	static const int ElementCount = 1;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[ElementCount];
};

