#pragma once

#include <DirectXMath.h>
#include "Lights.h"
#include <vector>
#include <VertexTypes.h>

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			throw std::exception();
		}
	}
}

namespace Helpers
{
	inline DirectX::XMMATRIX ComputeInverseTranspose(DirectX::FXMMATRIX matrix)
	{
		DirectX::XMMATRIX A = matrix;
		A.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR det = DirectX::XMMatrixDeterminant(A);
		
		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&det, A));
	}

	inline DirectX::XMFLOAT4X4 ComputeInverseTranspose(const DirectX::XMFLOAT4X4& matrix)
	{
		DirectX::XMMATRIX m = ComputeInverseTranspose(DirectX::XMLoadFloat4x4(&matrix));	
		DirectX::XMFLOAT4X4 f;
		DirectX::XMStoreFloat4x4(&f, m);
		
		return f;
	}

	inline DirectX::XMFLOAT4X4 XMMatrixToStorage(DirectX::FXMMATRIX m)
	{
		DirectX::XMFLOAT4X4 f;
		XMStoreFloat4x4(&f, m);

		return f;
	}

	template<typename T>
	inline void CreateMeshBuffer(ID3D11Device* device, std::vector<T> data, D3D11_BIND_FLAG bindFlag, ID3D11Buffer** outppBuffer)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.ByteWidth = static_cast<UINT>(sizeof(T) * data.size());
		desc.BindFlags = bindFlag;
		
		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data.data();
		
		DX::ThrowIfFailed(device->CreateBuffer(&desc, &initData, outppBuffer));
	}

	template<typename T>
	inline void CreateMeshBuffer(ID3D11Device* device, T* data, UINT numElements, D3D11_BIND_FLAG bindFlag, ID3D11Buffer** outppBuffer)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_IMMUTABLE; 
		desc.ByteWidth = sizeof(T) * numElements;
		desc.BindFlags = bindFlag;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;

		DX::ThrowIfFailed(device->CreateBuffer(&desc, &initData, outppBuffer));
	}

	/* tessellation is number of vertices in each dimension
	/  so for tessellation of 8, 8*8 vertices will be generated */

	void CreateGridXZ(std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices,
		const uint16_t width, const uint16_t depth, uint16_t tessellation = 32);

	void CreateGridXY(std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices,
		const uint16_t width, const uint16_t depth, uint16_t tessellation = 32);
};



