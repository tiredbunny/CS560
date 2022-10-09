#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders\Compiled\BasicSkinnedVS.h"
#include "Shaders\Compiled\BasicPS.h"
}

using namespace DirectX;

void BasicSkinnedEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_SkinnedVS, sizeof(g_SkinnedVS), g_BasicPS, sizeof(g_BasicPS));
	m_CbPerFrame.Create(device);
	m_CbPerObject.Create(device);
	m_CbSkinned.Create(device);
}

void BasicSkinnedEffect::SetBoneTransforms(DirectX::XMFLOAT4X4* matrices, int count)
{
	//init everything to identity first -- not sure if necessary
	for (int i = 0; i < 96; ++i)
		m_CbSkinnedData.BoneTransforms[i] = Helpers::XMMatrixToStorage(XMMatrixIdentity());

	for (int i = 0; i < count; ++i)
	{
		m_CbSkinnedData.BoneTransforms[i] = matrices[i];
	}
}
void BasicSkinnedEffect::SetWorld(DirectX::FXMMATRIX world)
{
	m_CbPerObjectData.World = Helpers::XMMatrixToStorage(world);
	XMStoreFloat4x4(&m_CbPerObjectData.WorldInvTranspose, Helpers::ComputeInverseTranspose(world));
}

void BasicSkinnedEffect::SetWorldViewProj(DirectX::FXMMATRIX worldViewProj)
{
	m_CbPerObjectData.WorldViewProj = Helpers::XMMatrixToStorage(worldViewProj);
}

void BasicSkinnedEffect::SetTextureTransform(DirectX::FXMMATRIX texTransform)
{
	m_CbPerObjectData.TextureTransform = Helpers::XMMatrixToStorage(texTransform);
}

void BasicSkinnedEffect::SetMaterial(const Material& mat)
{
	m_CbPerObjectData.Material = mat;
}

void BasicSkinnedEffect::SetDirectionalLight(const DirectionalLight& light)
{
	m_CbPerFrameData.DirLight = light;
}

void BasicSkinnedEffect::SetPointLight(const PointLight& light)
{
	m_CbPerFrameData.PointLight = light;
}

void BasicSkinnedEffect::SetSpotLight(const SpotLight& light)
{
	m_CbPerFrameData.SpotLight = light;
}

void BasicSkinnedEffect::SetEyePosition(DirectX::FXMVECTOR eyePos)
{
	XMStoreFloat3(&m_CbPerFrameData.EyePos, eyePos);
}

void BasicSkinnedEffect::SetFog(const FogProperties& fog)
{
	m_CbPerFrameData.Fog = fog;
}

void BasicSkinnedEffect::SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	context->PSSetSamplers(0, 1, &sampler);
}

void BasicSkinnedEffect::SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(0, 1, &srv);
}

void BasicSkinnedEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);

	context->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	context->VSSetConstantBuffers(1, 1, m_CbSkinned.GetAddressOf());


	context->PSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	context->PSSetConstantBuffers(1, 1, m_CbPerFrame.GetAddressOf());
}

void BasicSkinnedEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbPerObjectData);
	m_CbSkinned.SetData(context, m_CbSkinnedData);
}

void BasicSkinnedEffect::ApplyPerFrameConstants(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}
