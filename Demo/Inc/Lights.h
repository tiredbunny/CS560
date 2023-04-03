#pragma once

#include <DirectXMath.h>

struct DirectionalLight
{
	DirectX::XMFLOAT3 Direction;

	DirectionalLight()
	{
		SetDirection(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f));
	}

	void SetDirection(DirectX::FXMVECTOR direction)
	{
		DirectX::XMVECTOR v = DirectX::XMVector3Normalize(direction);
		DirectX::XMStoreFloat3(&Direction, v);
	}

	void SetDirection(const DirectX::XMFLOAT3& direction)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&direction);
		SetDirection(v);
	}
};



