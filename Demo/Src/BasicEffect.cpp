#include "pch.h"
#include "Shaders.h"

namespace
{
	#include "Shaders\Compiled\BasicPS.h"
	#include "Shaders\Compiled\BasicVS.h"
}

using namespace DirectX;

void BasicEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_BasicVS, sizeof(g_BasicVS), g_BasicPS, sizeof(g_BasicPS));
	m_CbPerFrame.Create(device);
	m_CbPerObject.Create(device);
}

void BasicEffect::SetWorld(DirectX::FXMMATRIX world)
{
	m_CbPerObjectData.World = Helpers::XMMatrixToStorage(world);
	XMStoreFloat4x4(&m_CbPerObjectData.WorldInvTranspose, Helpers::ComputeInverseTranspose(world));
}

void BasicEffect::SetWorldViewProj(DirectX::FXMMATRIX worldViewProj)
{
	m_CbPerObjectData.WorldViewProj = Helpers::XMMatrixToStorage(worldViewProj);
}

void BasicEffect::SetTextureTransform(DirectX::FXMMATRIX texTransform)
{
	m_CbPerObjectData.TextureTransform = Helpers::XMMatrixToStorage(texTransform);
}

void BasicEffect::SetMaterial(const Material& mat)
{
	m_CbPerObjectData.Material = mat;
}

void BasicEffect::SetDirectionalLight(const DirectionalLight& light)
{
	m_CbPerFrameData.DirLight = light;
}

void BasicEffect::SetPointLight(const PointLight& light)
{
	m_CbPerFrameData.PointLight = light;
}

void BasicEffect::SetSpotLight(const SpotLight& light)
{
	m_CbPerFrameData.SpotLight = light;
}

void BasicEffect::SetEyePosition(DirectX::FXMVECTOR eyePos)
{
	XMStoreFloat3(&m_CbPerFrameData.EyePos, eyePos);
}

void BasicEffect::SetFog(const FogProperties& fog)
{
	m_CbPerFrameData.Fog = fog;
}

void BasicEffect::SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	context->PSSetSamplers(0, 1, &sampler);
}

void BasicEffect::SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(0, 1, &srv);
}

void BasicEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);

	context->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	context->PSSetConstantBuffers(1, 1, m_CbPerFrame.GetAddressOf());
}

void BasicEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbPerObjectData);
}

void BasicEffect::ApplyPerFrameConstants(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}
