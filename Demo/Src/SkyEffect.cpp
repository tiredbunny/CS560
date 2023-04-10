#include "pch.h"
#include "Shaders.h"

namespace
{
#include "Shaders/Compiled/CompiledSkyPS.h"
#include "Shaders/Compiled/CompiledSkyVS.h"
}

void SkyEffect::Create(ID3D11Device* device, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& inputLayout)
{
	PipelineShaderObjects::Create(device, inputLayout, g_SkyVS, sizeof(g_SkyVS), g_SkyPS, sizeof(g_SkyPS));
	m_CbPerObject.Create(device);
}

void SkyEffect::SetWorldViewProj(DirectX::FXMMATRIX worldViewProj)
{
	m_CbConstantsData.WorldViewProj = Helpers::XMMatrixToStorage(worldViewProj);
}

void SkyEffect::SetTextureCube(ID3D11DeviceContext* context, ID3D11ShaderResourceView* srv)
{
	context->PSSetShaderResources(0, 1, &srv);
}

void SkyEffect::SetSamplerState(ID3D11DeviceContext* context, ID3D11SamplerState* state)
{
	context->PSSetSamplers(0, 1, &state);
}

void SkyEffect::Apply(ID3D11DeviceContext* context)
{
	m_CbPerObject.SetData(context, m_CbConstantsData);
}

void SkyEffect::Bind(ID3D11DeviceContext* context) const
{
	PipelineShaderObjects::Bind(context);
	context->VSSetConstantBuffers(0, 1, m_CbPerObject.GetAddressOf());
}
