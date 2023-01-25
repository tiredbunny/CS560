#pragma once

struct SkyVertex
{
	DirectX::XMFLOAT3 Position;

	static const int ElementCount = 1;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[ElementCount];
};

struct ScreenQuadVertex
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT2 TexCoord;

	static const int ElementCount = 2;
	static const D3D11_INPUT_ELEMENT_DESC InputElements[ElementCount];
};