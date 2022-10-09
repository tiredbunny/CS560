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
	inline void CreateStructuredBuffer(ID3D11Device* device, std::vector<T> data, UINT bindFlag, ID3D11Buffer** outppBuffer,
		UINT cpuFlags = 0, D3D11_USAGE usage = D3D11_USAGE_DEFAULT)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = usage;
		desc.ByteWidth = static_cast<UINT>(sizeof(T) * data.size());
		desc.BindFlags = bindFlag;
		desc.StructureByteStride = sizeof(T);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = cpuFlags;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data.data();

		DX::ThrowIfFailed(device->CreateBuffer(&desc, &initData, outppBuffer));
	}

	template<typename T>
	inline void CreateStructuredBuffer(ID3D11Device* device, UINT numElements, UINT bindFlag, ID3D11Buffer** outppBuffer,
		UINT cpuFlags = 0, D3D11_USAGE usage = D3D11_USAGE_DEFAULT)
	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = usage;
		desc.ByteWidth = static_cast<UINT>(sizeof(T) * numElements);
		desc.BindFlags = bindFlag;
		desc.StructureByteStride = sizeof(T);
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.CPUAccessFlags = cpuFlags;

		DX::ThrowIfFailed(device->CreateBuffer(&desc, nullptr, outppBuffer));
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

	inline DirectionalLight GetReflectedLight(const DirectionalLight& light, DirectX::FXMMATRIX reflectionMatrix)
	{
		DirectionalLight reflected = light;
		DirectX::XMVECTOR refDirection =  XMVector3TransformNormal(DirectX::XMLoadFloat3(&light.Direction), reflectionMatrix);
		XMStoreFloat3(&reflected.Direction, refDirection);
		
		return reflected;
	}

	inline PointLight GetReflectedLight(const PointLight& light, DirectX::FXMMATRIX reflectionMatrix)
	{
		PointLight reflected = light;
		DirectX::XMVECTOR refPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&light.Position), reflectionMatrix);
		XMStoreFloat3(&reflected.Position, refPosition);

		return reflected;
	}

	inline SpotLight GetReflectedLight(const SpotLight& light, DirectX::FXMMATRIX reflectionMatrix)
	{
		SpotLight reflected = light;
		
		DirectX::XMVECTOR refPosition = DirectX::XMVector3TransformCoord(DirectX::XMLoadFloat3(&light.Position), reflectionMatrix);
		DirectX::XMVECTOR refDirection = XMVector3TransformNormal(DirectX::XMLoadFloat3(&light.Direction), reflectionMatrix);
		
		XMStoreFloat3(&reflected.Position, refPosition);
		XMStoreFloat3(&reflected.Direction, refDirection);

		return reflected;
	}


	/* tessellation is number of vertices in each dimension
	/  so for tessellation of 8, 8*8 vertices will be generated */

	void CreateGrid(std::vector<DirectX::VertexPositionNormalTexture>& vertices, std::vector<uint16_t>& indices,
		const uint16_t width, const uint16_t depth, uint16_t tessellation = 32);
};



