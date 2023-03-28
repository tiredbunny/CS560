#include "pch.h"
#include "Shaders.h"

namespace
{
	#include "Shaders\Compiled\BasicPS.h"
	#include "Shaders\Compiled\BasicVS.h"
}

using namespace DirectX;

void RenderGBuffersEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_BasicVS, sizeof(g_BasicVS), g_BasicPS, sizeof(g_BasicPS));
	m_CbPerFrame.Create(device);
	m_CbPerObject.Create(device);
}

void RenderGBuffersEffect::SetWorld(DirectX::FXMMATRIX world)
{
	m_CbPerObjectData.World = Helpers::XMMatrixToStorage(world);
	XMStoreFloat4x4(&m_CbPerObjectData.WorldInvTranspose, Helpers::ComputeInverseTranspose(world));
}

void RenderGBuffersEffect::SetWorldViewProj(DirectX::FXMMATRIX worldViewProj)
{
	m_CbPerObjectData.WorldViewProj = Helpers::XMMatrixToStorage(worldViewProj);
}

void RenderGBuffersEffect::SetTextureTransform(DirectX::FXMMATRIX texTransform)
{
	m_CbPerObjectData.TextureTransform = Helpers::XMMatrixToStorage(texTransform);
}

void RenderGBuffersEffect::SetShadowTransform(DirectX::FXMMATRIX transform)
{
	m_CbPerObjectData.ShadowTransform = Helpers::XMMatrixToStorage(transform);
}

void RenderGBuffersEffect::SetShadowSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	context->PSSetSamplers(1, 1, &sampler);
}

void RenderGBuffersEffect::EnableMomentShadowMap(bool flag)
{
	if (flag)
		m_CbPerFrameData.ShadowMethod = 1.0f;
	else
		m_CbPerFrameData.ShadowMethod = 0.0f;
}

void RenderGBuffersEffect::SetSampler(ID3D11DeviceContext* context, ID3D11SamplerState* sampler)
{
	context->PSSetSamplers(0, 1, &sampler);
}

void RenderGBuffersEffect::SetTexture(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(0, 1, &srv);
}

void RenderGBuffersEffect::SetShadowMap(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(1, 1, &srv);
}

void RenderGBuffersEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);

	context->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
	context->PSSetConstantBuffers(0, 1, m_CbPerFrame.GetAddressOf());
}

void RenderGBuffersEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbPerObjectData);
}

void RenderGBuffersEffect::ApplyPerFrameConstants(ID3D11DeviceContext* context)
{
	m_CbPerFrame.SetData(context, m_CbPerFrameData);
}
