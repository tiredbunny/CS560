#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders/Compiled/ShadowMapVS.h"
#include "Shaders/Compiled/ShadowMapPS.h"
}

using namespace DirectX;

void ShadowMapEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_ShadowMapVS, sizeof(g_ShadowMapVS),
		g_ShadowMapPS, sizeof(g_ShadowMapPS));
	m_CbPerObject.Create(device);
}

void ShadowMapEffect::SetWorldViewProj(DirectX::FXMMATRIX WorldViewProj)
{
	m_CbPerObjectData.WorldViewProj = Helpers::XMMatrixToStorage(WorldViewProj);
}

void ShadowMapEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbPerObjectData);
}

void ShadowMapEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);
	context->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
}