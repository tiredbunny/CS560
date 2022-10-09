#pragma once

#include <d3d11.h>
#include <wrl\client.h>
#include "Helpers.h"
#include "ConstantBuffer.h"

class PipelineShaderObjects
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> m_GeometryShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
public:
	PipelineShaderObjects() = default;
	PipelineShaderObjects(PipelineShaderObjects const&) = delete;
	PipelineShaderObjects& operator=(PipelineShaderObjects const&) = delete;

public:
	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout,
		const void* pVertexShaderByteCode, SIZE_T vertexShaderByteLength,
		const void* pPixelShaderByteCode, SIZE_T pixelShaderByteLength) 
	{
		assert(device != nullptr);
		
		m_InputLayout = inputLayout;
		DX::ThrowIfFailed(device->CreateVertexShader(pVertexShaderByteCode, vertexShaderByteLength, nullptr, m_VertexShader.ReleaseAndGetAddressOf()));
		DX::ThrowIfFailed(device->CreatePixelShader(pPixelShaderByteCode, pixelShaderByteLength, nullptr, m_PixelShader.ReleaseAndGetAddressOf()));
	}


	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout,
		const void* pVertexShaderByteCode, SIZE_T vertexShaderByteLength,
		const void* pPixelShaderByteCode, SIZE_T pixelShaderByteLength,
		const void* pGeometryShaderByteCode, SIZE_T geometryShaderByteLength)
	{
		Create(device, inputLayout, pVertexShaderByteCode, vertexShaderByteLength, pPixelShaderByteCode, pixelShaderByteLength);

		DX::ThrowIfFailed(device->CreateGeometryShader(pGeometryShaderByteCode, geometryShaderByteLength, nullptr, m_GeometryShader.ReleaseAndGetAddressOf()));
	}

	void Bind(ID3D11DeviceContext* context) const
	{
		assert(context != nullptr);

		context->IASetInputLayout(m_InputLayout.Get());
		context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
		context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
		context->GSSetShader(m_GeometryShader.Get(), nullptr, 0);
	}
};

class BasicLightsEffect : PipelineShaderObjects
{
private:
	struct VS_PS_CbPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldInvTranspose;
		DirectX::XMFLOAT4X4 TextureTransform;
		Material Material;

	} m_CbPerObjectData;

	struct PS_CbPerFrame
	{
		DirectionalLight DirLight;
		PointLight PointLight;
		SpotLight SpotLight;

		DirectX::XMFLOAT3 EyePos;
		float pad;

		FogProperties Fog;

	} m_CbPerFrameData;

	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");
	static_assert(sizeof(VS_PS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<VS_PS_CbPerObject> m_CbPerObject;
	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;
public:
	BasicLightsEffect() = default;
	BasicLightsEffect(const BasicLightsEffect&) = delete;
	BasicLightsEffect& operator=(const BasicLightsEffect&) = delete;

	BasicLightsEffect(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout) 
	{ 
		Create(device, inputLayout); 
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetWorld(DirectX::FXMMATRIX world);
	void SetWorldViewProj(DirectX::FXMMATRIX worldViewProj);
	void SetTextureTransform(DirectX::FXMMATRIX texTransform);
	void SetMaterial(const Material& mat);
	void Apply(ID3D11DeviceContext* context);

	void SetDirectionalLight(const DirectionalLight& light);
	void SetPointLight(const PointLight& light);
	void SetSpotLight(const SpotLight& light);
	void SetEyePosition(DirectX::FXMVECTOR eyePos);
	void SetFog(const FogProperties& fog);
	void ApplyPerFrameConstants(ID3D11DeviceContext* context);
	
	void SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);

	void Bind(ID3D11DeviceContext* context) const;
};


//cbuffer cbSkinned : register(b1)
//{
//	float4x4 gBoneTransforms[96];
//};

class BasicSkinnedEffect : PipelineShaderObjects
{
private:
	struct VS_PS_CbPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldInvTranspose;
		DirectX::XMFLOAT4X4 TextureTransform;
		Material Material;

	} m_CbPerObjectData;

	struct VS_CbSkinned
	{
		DirectX::XMFLOAT4X4 BoneTransforms[96];
	}m_CbSkinnedData;

	struct PS_CbPerFrame
	{
		DirectionalLight DirLight;
		PointLight PointLight;
		SpotLight SpotLight;

		DirectX::XMFLOAT3 EyePos;
		float pad;

		FogProperties Fog;

	} m_CbPerFrameData;

	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");
	static_assert(sizeof(VS_PS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");
	static_assert(sizeof(VS_CbSkinned) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<VS_PS_CbPerObject> m_CbPerObject;
	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;
	ConstantBuffer<VS_CbSkinned> m_CbSkinned;
public:
	BasicSkinnedEffect() = default;
	BasicSkinnedEffect(const BasicLightsEffect&) = delete;
	BasicSkinnedEffect& operator=(const BasicLightsEffect&) = delete;

	BasicSkinnedEffect(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetBoneTransforms(DirectX::XMFLOAT4X4* matrices, int count);
	void SetWorld(DirectX::FXMMATRIX world);
	void SetWorldViewProj(DirectX::FXMMATRIX worldViewProj);
	void SetTextureTransform(DirectX::FXMMATRIX texTransform);
	void SetMaterial(const Material& mat);
	void Apply(ID3D11DeviceContext* context);

	void SetDirectionalLight(const DirectionalLight& light);
	void SetPointLight(const PointLight& light);
	void SetSpotLight(const SpotLight& light);
	void SetEyePosition(DirectX::FXMVECTOR eyePos);
	void SetFog(const FogProperties& fog);
	void ApplyPerFrameConstants(ID3D11DeviceContext* context);

	void SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);

	void Bind(ID3D11DeviceContext* context) const;
};

