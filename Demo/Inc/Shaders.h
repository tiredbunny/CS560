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
		
		if (pixelShaderByteLength != 0)
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

class RenderGBuffersEffect : PipelineShaderObjects
{
private:
	struct VS_CbPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4X4 World;
		DirectX::XMFLOAT4X4 WorldInvTranspose;
		DirectX::XMFLOAT4X4 TextureTransform;
		DirectX::XMFLOAT4X4 ShadowTransform;

	} m_CbPerObjectData;

	struct PS_CbPerFrame
	{
		float ShadowMethod;
		float BloomBrightness;
		DirectX::XMFLOAT2 pad;

		float metallic;
		float roughness;
		float ao;
		float gammaExposure;

		DirectX::XMFLOAT4 SolidColor;
	} m_CbPerFrameData;

	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");
	static_assert(sizeof(VS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<VS_CbPerObject> m_CbPerObject;
	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;
public:
	RenderGBuffersEffect() = default;
	RenderGBuffersEffect(const RenderGBuffersEffect&) = delete;
	RenderGBuffersEffect& operator=(const RenderGBuffersEffect&) = delete;

	RenderGBuffersEffect(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout) 
	{ 
		Create(device, inputLayout); 
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetWorld(DirectX::FXMMATRIX world);
	void SetWorldViewProj(DirectX::FXMMATRIX worldViewProj);
	void SetTextureTransform(DirectX::FXMMATRIX texTransform);
	void SetPBRProperties(float metallic, float roughness, float ao, float gammaExposure);
	void SetSolidColor(DirectX::XMFLOAT4 color);
	void SetShadowTransform(DirectX::FXMMATRIX shadowTransform);
	void SetShadowSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void SetBloomExtractBrightness(float brightness);
	void EnableMomentShadowMap(bool flag);

	void Apply(ID3D11DeviceContext* context);
	void ApplyPerFrameConstants(ID3D11DeviceContext* context);
	
	void SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);
	void SetShadowMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);

	void Bind(ID3D11DeviceContext* context) const;
};

class LocalLightEffect : PipelineShaderObjects
{
	struct VS_CbPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;

	} m_CbPerObjectData;
	
	struct PS_CbPerFrame
	{
		DirectX::XMFLOAT3 CameraPosition;
		float pad1;

		DirectX::XMFLOAT3 LightColor;
		float visualizeSphere;

		DirectX::XMFLOAT3 LightPosition;
		float radius;

	} m_CbPerFrameData;

	static_assert(sizeof(VS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");
	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<VS_CbPerObject> m_CbPerObject;
	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;
public:
	LocalLightEffect() = default;
	LocalLightEffect(const LocalLightEffect&) = delete;
	LocalLightEffect& operator=(const LocalLightEffect&) = delete;

	LocalLightEffect(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}
	
	void SetWorldViewProj(DirectX::FXMMATRIX worldViewProj);
	void SetCameraPosition(DirectX::XMFLOAT3 position);
	void SetLightData(DirectX::XMFLOAT3 lightPos, DirectX::XMFLOAT3 lightColor, float radius);
	void SetVisualizeSphere(bool enabled);
	void SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv);
	
	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);
	void Apply(ID3D11DeviceContext* context);
	void ApplyPerFrameConstants(ID3D11DeviceContext* context);
	void Bind(ID3D11DeviceContext* context) const;
};

class HammerseleyData;

class ScreenQuadEffect : PipelineShaderObjects
{
	struct PS_CbPerFrame
	{
		DirectX::XMFLOAT3 LightDir;
		float pad1;
		DirectX::XMFLOAT3 LightColor;
		float pad2;
		DirectX::XMFLOAT3 CameraPosition;
		float pad3;

		float width;
		float height;
		float N;
		float useAO;

		float hammersley[2 * 96];
	} m_CbPerFrameData;

	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;
public:
	ScreenQuadEffect() = default;
	ScreenQuadEffect(const ScreenQuadEffect&) = delete;
	ScreenQuadEffect& operator=(const ScreenQuadEffect&) = delete;

	ScreenQuadEffect(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);
	void SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv);

	void SetScreenResolution(float width, float height);
	void SetIRMapAndEnvMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* IRMap, ID3D11ShaderResourceView* EnvMap);
	void SetAOMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);
	void SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void EnableAmbientOcclusion(bool enabled);
	void SetHammersleyData(HammerseleyData data);
	void SetCameraPosition(DirectX::XMFLOAT3 camPos);
	void SetGlobalLight(DirectX::XMFLOAT3 lightDir, DirectX::XMFLOAT3 lightColor);

	void Apply(ID3D11DeviceContext* context);
	void Bind(ID3D11DeviceContext* context) const;
};

class AOScreenQuadEffect : PipelineShaderObjects
{
	//cbuffer cbPerFrame : register(b0)
	//{
	//	float3 eyePos;
	//	float s;

	//	float width;
	//	float height;
	//	float k;
	//	float R;
	//}
	struct PS_CbPerFrame
	{
		DirectX::XMFLOAT3 eyePos;
		float s = 1.0f;

		float width;
		float height;
		float k = 1.0f;
		float R = 0.5f;
	} m_CbPerFrameData;

	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");
	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;
public:
	AOScreenQuadEffect() = default;
	AOScreenQuadEffect(const AOScreenQuadEffect&) = delete;
	AOScreenQuadEffect& operator=(const AOScreenQuadEffect&) = delete;

	AOScreenQuadEffect(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);
	void SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv);
	void SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler);
	void SetAOData(float s, float k, float R);
	void SetScreenResolution(float width, float height);
	void SetCameraPosition(DirectX::XMFLOAT3 camPos);

	void Apply(ID3D11DeviceContext* context);
	void Bind(ID3D11DeviceContext* context) const;
};

class AOBlurEffect : PipelineShaderObjects
{
	/*cbuffer cbPerFrame : register(b0)
	{
		float4 gBlurWeights[3];

		float gHorizontalBlur;
		float width;
		float height;
		float pad;
	}*/
	struct PS_CbPerFrame
	{
		DirectX::XMFLOAT4 BlurWeights[3];

		float HorizontalBlur; // > 0 = true
		float width;
		float height;
		float preserveEdge;

		DirectX::XMMATRIX Proj;
	} m_CbPerFrameData;

	static_assert(sizeof(PS_CbPerFrame) % 16 == 0, "struct not 16-byte aligned");
	ConstantBuffer<PS_CbPerFrame> m_CbPerFrame;

	std::vector<float> m_BlurWeights;
public:
	AOBlurEffect() = default;
	AOBlurEffect(const AOBlurEffect&) = delete;
	AOBlurEffect& operator=(const AOBlurEffect&) = delete;

	AOBlurEffect(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}
	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);
	void SetGBuffers(ID3D11DeviceContext* context, int bufferCount, ID3D11ShaderResourceView** srv);
	void SetInputMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);
	void SetBlurWeights(float sigma);
	void SetHorizontalBlur(bool isHorizontal);
	void SetScreenResolution(float width, float height);
	void SetProjMatrix(DirectX::CXMMATRIX proj);
	void SetBlurEdgePreserve(bool isEdgePreserving);

	void Apply(ID3D11DeviceContext* context);
	void Bind(ID3D11DeviceContext* context) const;

	void CalculateBlurWeights(float sigma);
};

class SkyEffect : PipelineShaderObjects
{
	struct VS_CbPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
	} m_CbConstantsData;


	static_assert(sizeof(VS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");
	ConstantBuffer<VS_CbPerObject> m_CbPerObject;
public:
	SkyEffect() = default;
	SkyEffect(const SkyEffect&) = delete;
	SkyEffect& operator=(const SkyEffect&) = delete;

	SkyEffect(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}

	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetWorldViewProj(DirectX::FXMMATRIX worldViewProj);
	void SetTextureCube(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv);
	void SetSamplerState(ID3D11DeviceContext* context, ID3D11SamplerState* state);

	void Apply(ID3D11DeviceContext* context);
	void Bind(ID3D11DeviceContext* context) const;
};

class ShadowMapEffect : PipelineShaderObjects
{
private:
	struct VS_CbPerObject
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
	} m_CbPerObjectData;

	static_assert(sizeof(VS_CbPerObject) % 16 == 0, "struct not 16-byte aligned");

	ConstantBuffer<VS_CbPerObject> m_CbPerObject;
public:
	ShadowMapEffect() = default;
	ShadowMapEffect(const ShadowMapEffect&) = delete;
	ShadowMapEffect& operator=(const ShadowMapEffect&) = delete;

	ShadowMapEffect(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
	{
		Create(device, inputLayout);
	}
	void Create(ID3D11Device* device, const	Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout);

	void SetWorldViewProj(DirectX::FXMMATRIX WorldViewProj);
	void Apply(ID3D11DeviceContext* context);
	void Bind(ID3D11DeviceContext* context) const;
};